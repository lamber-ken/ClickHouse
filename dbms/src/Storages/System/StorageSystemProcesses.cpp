#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypesNumber.h>
#include <Interpreters/Context.h>
#include <Interpreters/ProcessList.h>
#include <Storages/System/StorageSystemProcesses.h>


namespace DB
{

NamesAndTypesList StorageSystemProcesses::getNamesAndTypes()
{
    return {
        {"is_initial_query", std::make_shared<DataTypeUInt8>()},

        {"user", std::make_shared<DataTypeString>()},
        {"query_id", std::make_shared<DataTypeString>()},
        {"address", std::make_shared<DataTypeString>()},
        {"port", std::make_shared<DataTypeUInt16>()},

        {"initial_user", std::make_shared<DataTypeString>()},
        {"initial_query_id", std::make_shared<DataTypeString>()},
        {"initial_address", std::make_shared<DataTypeString>()},
        {"initial_port", std::make_shared<DataTypeUInt16>()},

        {"interface", std::make_shared<DataTypeUInt8>()},

        {"os_user", std::make_shared<DataTypeString>()},
        {"client_hostname", std::make_shared<DataTypeString>()},
        {"client_name", std::make_shared<DataTypeString>()},
        {"client_version_major", std::make_shared<DataTypeUInt64>()},
        {"client_version_minor", std::make_shared<DataTypeUInt64>()},
        {"client_version_patch", std::make_shared<DataTypeUInt64>()},
        {"client_revision", std::make_shared<DataTypeUInt64>()},

        {"http_method", std::make_shared<DataTypeUInt8>()},
        {"http_user_agent", std::make_shared<DataTypeString>()},

        {"quota_key", std::make_shared<DataTypeString>()},

        {"elapsed", std::make_shared<DataTypeFloat64>()},
        {"is_cancelled", std::make_shared<DataTypeUInt8>()},
        {"read_rows", std::make_shared<DataTypeUInt64>()},
        {"read_bytes", std::make_shared<DataTypeUInt64>()},
        {"total_rows_approx", std::make_shared<DataTypeUInt64>()},
        {"written_rows", std::make_shared<DataTypeUInt64>()},
        {"written_bytes", std::make_shared<DataTypeUInt64>()},
        {"memory_usage", std::make_shared<DataTypeInt64>()},
        {"peak_memory_usage", std::make_shared<DataTypeInt64>()},
        {"query", std::make_shared<DataTypeString>()},
    };
}


void StorageSystemProcesses::fillData(MutableColumns & res_columns, const Context & context, const SelectQueryInfo &) const
{
    ProcessList::Info info = context.getProcessList().getInfo();

    for (const auto & process : info)
    {
        size_t i = 0;
        res_columns[i++]->insert(UInt64(process.client_info.query_kind == ClientInfo::QueryKind::INITIAL_QUERY));
        res_columns[i++]->insert(process.client_info.current_user);
        res_columns[i++]->insert(process.client_info.current_query_id);
        res_columns[i++]->insert(process.client_info.current_address.host().toString());
        res_columns[i++]->insert(UInt64(process.client_info.current_address.port()));
        res_columns[i++]->insert(process.client_info.initial_user);
        res_columns[i++]->insert(process.client_info.initial_query_id);
        res_columns[i++]->insert(process.client_info.initial_address.host().toString());
        res_columns[i++]->insert(UInt64(process.client_info.initial_address.port()));
        res_columns[i++]->insert(UInt64(process.client_info.interface));
        res_columns[i++]->insert(process.client_info.os_user);
        res_columns[i++]->insert(process.client_info.client_hostname);
        res_columns[i++]->insert(process.client_info.client_name);
        res_columns[i++]->insert(process.client_info.client_version_major);
        res_columns[i++]->insert(process.client_info.client_version_minor);
        res_columns[i++]->insert(process.client_info.client_version_patch);
        res_columns[i++]->insert(UInt64(process.client_info.client_revision));
        res_columns[i++]->insert(UInt64(process.client_info.http_method));
        res_columns[i++]->insert(process.client_info.http_user_agent);
        res_columns[i++]->insert(process.client_info.quota_key);
        res_columns[i++]->insert(process.elapsed_seconds);
        res_columns[i++]->insert(UInt64(process.is_cancelled));
        res_columns[i++]->insert(UInt64(process.read_rows));
        res_columns[i++]->insert(UInt64(process.read_bytes));
        res_columns[i++]->insert(UInt64(process.total_rows));
        res_columns[i++]->insert(UInt64(process.written_rows));
        res_columns[i++]->insert(UInt64(process.written_bytes));
        res_columns[i++]->insert(process.memory_usage);
        res_columns[i++]->insert(process.peak_memory_usage);
        res_columns[i++]->insert(process.query);
    }
}

}
