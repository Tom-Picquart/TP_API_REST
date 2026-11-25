#include "UserService.h"

UserService::UserService(UserDB& userDB) : AbstractService("/user"), userDB_(userDB) {}

http::status UserService::get(json::value& out, std::optional<int> id) const
{
    // TODO Question 6 (et Question 13 pour GET /user)
    (void)out; (void)id;
    
    if (!id){
        throw ServiceException(http::status::bad_request,"Missing user id");
    }
    const User* user = userDB_.getUser(*id);

    if (user == nullptr){
        throw ServiceException(http::status::not_found,"Invalid user id");
    }

    out = json::value_from(*user);
    return http::status::ok;
}

http::status UserService::post(json::value& out, const json::value& in)
{
    // TODO Question 8
    (void)out; (void)in;

    if (!in.is_object()) {
        throw ServiceException(http::status::bad_request, "Expected a JSON object");
    }

    const json::object& obj = in.as_object();
    const json::value* nameValue = obj.if_contains("name");
    if (nameValue == nullptr || !nameValue->is_string()) {
        throw ServiceException(http::status::bad_request, "Missing or invalid name");
    }

    const boost::json::string& name = nameValue->as_string();
    if (name.empty()) {
        throw ServiceException(http::status::bad_request, "Name cannot be empty");
    }
    const json::value* ageValue = obj.if_contains("age");
    if (ageValue == nullptr || !ageValue->is_int64()) {
        throw ServiceException(http::status::bad_request, "Missing or invalid age");
    }
    std::int64_t age = ageValue->as_int64();
    if (age < 0 || age > 150) {
        throw ServiceException(http::status::bad_request, "Age must be between 0 and 150");
    }

    int id = userDB_.addUser({std::string(name.begin(), name.end()), static_cast<int>(age)});

    out = {{"id", id}};
    return http::status::created;
}


http::status UserService::patch(std::optional<int> id, const json::value& in)
{
    // TODO Question 10
    (void)id; (void)in;
    if (!id.has_value()) {
        throw ServiceException(http::status::method_not_allowed,"PATCH requires a user id");
    }

    const User* existingUser = userDB_.getUser(*id);

    if (existingUser == nullptr) {
        throw ServiceException(http::status::not_found,"User not found");
    }

    if (!in.is_object()) {
        throw ServiceException(http::status::bad_request,"Expected a JSON object");
    }

    User updatedUser = *existingUser;
    const json::object& obj = in.as_object();

    for (const auto& [key, value] : obj) {
        if (key == "name") {
            if (!value.is_string()) {
                throw ServiceException(http::status::bad_request,"Invalid name");
            }

            updatedUser.name = std::string(value.as_string().begin(),value.as_string().end());
            if (updatedUser.name.empty()) {
                throw ServiceException(http::status::bad_request,"Name cannot be empty");
            }
        }
        else if (key == "age") {
            if (!value.is_int64()) {
                throw ServiceException(http::status::bad_request,"Invalid age");
            }

            std::int64_t age = value.as_int64();

            if (age < 0 || age > 150) {
                throw ServiceException(http::status::bad_request,"Age must be between 0 and 150");
            }

            updatedUser.age = static_cast<int>(age);
        }
        else {
            throw ServiceException(http::status::bad_request,"Unknown field");
        }
    }

    if (!userDB_.setUser(*id, updatedUser)) {
        throw ServiceException(http::status::not_found,"User not found");
    }
    return http::status::no_content;
}


http::status UserService::remove(std::optional<int> id)
{
    // TODO Question 12
    (void)id;
    throw ServiceException(http::status::not_implemented, "Not implemented yet");
}
