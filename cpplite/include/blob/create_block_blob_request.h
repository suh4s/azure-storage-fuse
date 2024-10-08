#pragma once

#include "put_blob_request_base.h"

namespace azure { namespace storage_lite {

    class create_block_blob_request : public put_blob_request_base
    {
    public:
        create_block_blob_request(const std::string &container, const std::string &blob)
            : m_container(container),
              m_blob(blob),
              m_content_length(0) {}

        std::string container() const override
        {
            return m_container;
        }

        std::string blob() const override
        {
            return m_blob;
        }

        blob_type ms_blob_type() const override
        {
            return blob_type::block_blob;
        }

        std::string content_type() const override
        {
            return m_content_type;
        }

        create_block_blob_request &set_content_type(std::string content_type)
        {
            m_content_type = content_type;
            return *this;
        }

        unsigned int content_length() const override
        {
            return m_content_length;
        }

        create_block_blob_request &set_content_length(unsigned int content_length)
        {
            m_content_length = content_length;
            return *this;
        }

        std::vector<std::pair<std::string, std::string>> metadata() const override
        {
            return m_metadata;
        }

        create_block_blob_request &set_metadata(const std::vector<std::pair<std::string, std::string>> &metadata)
        {
            m_metadata = metadata;
            return *this;
        }


    private:
        std::string m_container;
        std::string m_blob;

        unsigned int m_content_length;
        std::string m_content_type;
        std::vector<std::pair<std::string, std::string>> m_metadata;
    };

    class create_append_blob_request final : public create_block_blob_request
    {
    public:
        create_append_blob_request(const std::string &container, const std::string &blob)
            : create_block_blob_request(container, blob) {}

        blob_type ms_blob_type() const override
        {
            return blob_type::append_blob;
        }

        unsigned int content_length() const override
        {
            return 0;
        }
    };

    class create_page_blob_request final : public create_block_blob_request {
    public:
        create_page_blob_request(const std::string &container, const std::string &blob, unsigned long long size)
            : create_block_blob_request(container, blob),
              m_ms_blob_content_length(size) {}

        blob_type ms_blob_type() const override
        {
            return blob_type::page_blob;
        }

        unsigned int content_length() const override
        {
            return 0;
        }

        unsigned long long ms_blob_content_length() const override
        {
            return m_ms_blob_content_length;
        }

    private:
        unsigned long long m_ms_blob_content_length;
    };

}}  // azure::storage_lite
