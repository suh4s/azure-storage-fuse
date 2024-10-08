#pragma once

#include <chrono>
#include <algorithm>
#include <math.h>

#include "storage_EXPORTS.h"

#include "http_base.h"
#include "utility.h"

namespace azure {  namespace storage_lite {

    class retry_info final
    {
    public:
        retry_info(bool should_retry, std::chrono::seconds interval)
            : m_should_retry(should_retry),
            m_interval(interval) {}

        bool should_retry() const
        {
            return m_should_retry;
        }

        std::chrono::seconds interval() const
        {
            return m_interval;
        }

    private:
        bool m_should_retry;
        std::chrono::seconds m_interval;
    };

    class retry_context final
    {
    public:
        retry_context()
            : m_numbers(0),
            m_result(0) {}

        retry_context(int numbers, http_base::http_code result)
            : m_numbers(numbers),
            m_result(result) {}

        int numbers() const
        {
            return m_numbers;
        }

        http_base::http_code result() const
        {
            return m_result;
        }

        void add_result(http_base::http_code result)
        {
            m_result = result;
            m_numbers++;
        }

    private:
        int m_numbers;
        http_base::http_code m_result;
    };

    class retry_policy_base
    {
    public:
        virtual ~retry_policy_base() {}
        virtual retry_info evaluate(const retry_context &context) const = 0;
    };

    // Default retry policy
    class retry_policy final : public retry_policy_base
    {
    public:
        retry_info evaluate(const retry_context& context) const override
        {
            const int max_retry_count = 3;
            if (context.numbers() <= max_retry_count && can_retry(context.result()))
            {
                return retry_info(true, std::chrono::seconds(0));
            }
            return retry_info(false, std::chrono::seconds(0));
        }

    private:
        bool can_retry(http_base::http_code code) const
        {
            return retryable(code);
        }
    };

    // Exponential retry policy
    class expo_retry_policy final : public retry_policy_base
    {
    public:
        retry_info evaluate(const retry_context& context) const override
        {
            if (context.numbers() == 0) {
                return retry_info(true, std::chrono::seconds(0));
            } else if (context.numbers() < 26 && can_retry(context.result())) {
                double delay = (pow(1.2, context.numbers()-1)-1);
                delay = std::min(delay, 60.0); // Maximum backoff delay of 1 minute
                delay *= (((double)rand())/RAND_MAX)/2 + 0.75;
                return retry_info(true, std::chrono::seconds((int)delay));
            }
            return retry_info(false, std::chrono::seconds(0));
        }

    private:
        bool can_retry(http_base::http_code code) const
        {
            return retryable(code);
        }
    };


    //#define CPPLITE_DEBUG_RETRY_POLICY 1
    // Configurable retry policy
    class flex_retry_policy final : public retry_policy_base
    {
    public:
        flex_retry_policy(int max_try, double max_timeout_seconds, double retry_delay) :
                    maxTryCount(max_try), maxTimeoutSeconds(max_timeout_seconds), retryDelay(retry_delay){}

        retry_info evaluate(const retry_context& context) const override
        {
            #ifndef CPPLITE_DEBUG_RETRY_POLICY
            if (context.numbers() == 0) {
                return retry_info(true, std::chrono::seconds(0));
            } else 
            #endif 
            if (context.numbers() < maxTryCount && can_retry(context.result())) {
                // retryDelay ^ try number : to make it exponential with every try
                #ifndef CPPLITE_DEBUG_RETRY_POLICY
                double delay = (pow(retryDelay, context.numbers()-1)-1);
                #else
                double delay = (pow(retryDelay, context.numbers()+1)-1);
                #endif

                // Cap max delay at configured time
                delay = std::min(delay, maxTimeoutSeconds); // Maximum backoff delay of 1 minute

                // Randomize the delay to some extent from here
                delay *= (((double)rand())/RAND_MAX)/2 + 0.75;

                return retry_info(true, std::chrono::seconds((int)delay));
            }
            return retry_info(false, std::chrono::seconds(0));
        }

    private:
        bool can_retry(http_base::http_code code) const
        {
            #ifndef CPPLITE_DEBUG_RETRY_POLICY
            return retryable(code);
            #else
            return code  == 200 ? true : true;
            #endif
        }

        int maxTryCount;
        double maxTimeoutSeconds;
        double retryDelay;

    };

    // No-retry policy
    class no_retry_policy final : public retry_policy_base
    {
    public:
        retry_info evaluate(const retry_context& context) const override
        {
            unused(context);
            return retry_info(false, std::chrono::seconds(0));
        }
    };

}}  // azure::storage_lite
