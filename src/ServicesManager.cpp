#include "ServicesManager.h"

#include <charconv>
#include <optional>
#include <utility>
#include <system_error>
#include <boost/json.hpp>
#include <boost/system/error_code.hpp>

void ServicesManager::registerService(std::unique_ptr<AbstractService> service)
{
    // TODO Question 3
    (void)service;
    services_.push_back(std::move(service));
}

AbstractService* ServicesManager::findService(std::string_view url) const
{
    // TODO Question 3
    (void)url;
    for (auto& service : services_)
    {
        std::string_view pattern = service->getPattern();

        if (url.starts_with(pattern) && (url.size() == pattern.size() || url[pattern.size()] == '/')){
            return service.get();
        }
    }
    return nullptr;
}

http::status ServicesManager::queryService(std::string& out, const std::string& in,
                                           std::string_view url, http::verb method)
{
    // TODO Questions 1, 3, 4, 5, 7, 9, 11
    (void)out; (void)in; (void)url; (void)method;
    
    auto pos = url.find('?');

    if (pos != std::string_view::npos)
        url = url.substr(0, pos);

    AbstractService* service = findService(url);

    if (service == nullptr)
        throw ServiceException(http::status::not_found, "Service not found");

    std::string_view pattern = service->getPattern();
    std::string_view text = url.substr(pattern.size());

    if (!text.empty() && text.front() == '/')
        text.remove_prefix(1);

    std::optional<int> id;

    if (!text.empty())
    {
        int value = 0;
        auto [ptr, ec] = std::from_chars(text.data(),text.data() + text.size(),value);

        if (ec != std::errc{} || ptr != text.data() + text.size())
        {
            throw ServiceException(http::status::bad_request,"Invalid user id");
        }

        id = value;
    }

    json::value result;
    http::status status;

    switch (method)
    {
        case http::verb::get:{
            status = service->get(result, id);
            break;
        }

    
        case http::verb::post:{
            if (id.has_value())
            {
                throw ServiceException(http::status::method_not_allowed,"POST is only allowed on collections");
            }

            if (in.empty())
            {
                throw ServiceException(http::status::bad_request,"Missing JSON body");
            }
            boost::system::error_code ec;
            json::value input = json::parse(in, ec);

            if (ec)
            {
                throw ServiceException(http::status::bad_request,"Invalid JSON");
            }
            status = service->post(result, input);
            break;
        }

        case http::verb::patch: {
            if (in.empty()) {
                throw ServiceException(http::status::bad_request,"Missing JSON body");
            }

            boost::system::error_code ec;
            json::value input = json::parse(in, ec);

            if (ec || !input.is_object()) {
                throw ServiceException(http::status::bad_request,"Invalid JSON object");
            }
            status = service->patch(id, input);
            break;
        }

        default:{
            throw ServiceException(http::status::not_implemented,"Not Implemented");
        }
    }

    if (status == http::status::no_content) {
        out.clear();
    } 
    else {
        out = result.is_null() ? "" : json::serialize(result);
    }

    return status;
}