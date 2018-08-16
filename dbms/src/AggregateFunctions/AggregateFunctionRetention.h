#pragma once

#include <iostream>
#include <sstream>
#include <unordered_set>
#include <Columns/ColumnsNumber.h>
#include <Columns/ColumnArray.h>
#include <DataTypes/DataTypeDateTime.h>
#include <DataTypes/DataTypesNumber.h>
#include <DataTypes/DataTypeArray.h>
#include <IO/ReadHelpers.h>
#include <IO/WriteHelpers.h>
#include <Common/ArenaAllocator.h>
#include <Common/typeid_cast.h>
#include <ext/range.h>
#include <bitset>

#include <AggregateFunctions/IAggregateFunction.h>


namespace DB
{
namespace ErrorCodes
{
extern const int NUMBER_OF_ARGUMENTS_DOESNT_MATCH;
extern const int TOO_MANY_ARGUMENTS_FOR_FUNCTION;
}

struct ComparePairFirst final
{
    template <typename T1, typename T2>
    bool operator()(const std::pair<T1, T2> & lhs, const std::pair<T1, T2> & rhs) const
    {
        return lhs.first < rhs.first;
    }
};

struct AggregateFunctionRetentionData
{
    static constexpr auto max_events = 32;

    using Timestamp = std::uint32_t;
    using Events = std::bitset<max_events>;

    Events events;

    void add(UInt8 event)
    {
        events.set(event);
    }

    void merge(const AggregateFunctionRetentionData & other)
    {
        events |= other.events;
    }

    void serialize(WriteBuffer & buf) const
    {
        UInt32 eventV = events.to_ulong();
        writeBinary(eventV, buf);
    }

    void deserialize(ReadBuffer & buf)
    {
        UInt32 eventV;
        readBinary(eventV, buf);
        events = eventV;
    }
};

/**
  * The max size of events is 32, that's enough for retention analytics
  *
  * Usage:
  * - Retention(cond1, cond2, cond3, ....)
  */
class AggregateFunctionRetention final
        : public IAggregateFunctionDataHelper<AggregateFunctionRetentionData, AggregateFunctionRetention>
{
private:
    UInt8 events_size;

public:
    String getName() const override
    {
        return "retention";
    }

    AggregateFunctionRetention(const DataTypes & arguments)
    {
        for (const auto i : ext::range(0, arguments.size()))
        {
            auto cond_arg = arguments[i].get();
            if (!typeid_cast<const DataTypeUInt8 *>(cond_arg))
                throw Exception{"Illegal type " + cond_arg->getName() + " of argument " + toString(i) + " of aggregate function "
                        + getName() + ", must be UInt8",
                        ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT};
        }

        events_size = arguments.size();
    }


    DataTypePtr getReturnType() const override
    {
        return std::make_shared<DataTypeArray>( std::make_shared<DataTypeUInt8>() );
    }

    void add(AggregateDataPtr place, const IColumn ** columns, const size_t row_num, Arena *) const override
    {
        for (const auto i : ext::range(0, events_size))
        {
            auto event = static_cast<const ColumnVector<UInt8> *>(columns[i])->getData()[row_num];
            if (event)
            {
                this->data(place).add(i);
                break;
            }
        }
    }

    void merge(AggregateDataPtr place, ConstAggregateDataPtr rhs, Arena *) const override
    {
        this->data(place).merge(this->data(rhs));
    }

    void serialize(ConstAggregateDataPtr place, WriteBuffer & buf) const override
    {
        this->data(place).serialize(buf);
    }

    void deserialize(AggregateDataPtr place, ReadBuffer & buf, Arena *) const override
    {
        this->data(place).deserialize(buf);
    }

    void insertResultInto(ConstAggregateDataPtr place, IColumn & to) const override
    {
        auto & data_to = static_cast<ColumnArray &>(to).getData();
        auto & offsets_to = static_cast<ColumnArray &>(to).getOffsets();

        const auto firstFlag = this->data(place).events.test(0);
        data_to.insert(firstFlag ?  Field(static_cast<UInt64>(1)) :  Field(static_cast<UInt64>(0)));
        for (const auto i : ext::range(1, events_size))
        {
            auto result = Field(static_cast<UInt64>(0));
            if (firstFlag && this->data(place).events.test(i))
                result = Field(static_cast<UInt64>(1));

            data_to.insert(result);
        }
        offsets_to.push_back(offsets_to.size() == 0 ?  events_size : offsets_to.back() +  events_size);
    }

    const char * getHeaderFilePath() const override
    {
        return __FILE__;
    }
};

}
