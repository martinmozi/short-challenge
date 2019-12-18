#include <iostream>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <fstream>

// std::function
// std::bind
// std::placeholders
// std::map
// std::make_pair

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/schema.h>
#include <rapidjson/prettywriter.h>

bool g_done = false;

//
// TEST COMMANDS
//
auto help_command = R"(
 {
  "command":"help",
  "payload": {
    "usage":"Enter json command in 'command':'<command>','payload': { // json payload of arguments }",
  }
 }
)";

auto exit_command = R"(
 {
  "command":"exit",
  "payload": {
     "reason":"Exiting program on user request."
  }
 }
)";

class Controller 
{
public:
    bool help(rapidjson::Value &payload)
    {
        std::cout << "Controller::help: command: ";

        // implement

        return true;
    }

    bool exit(rapidjson::Value &payload)
    {
        std::cout << "Controller::exit: command: \n";
        g_done = true;
        return true;
    }

    bool commandList(rapidjson::Value& payload)
    {
        std::cout << "Controller::commandList: command: \n";
        payload["result"] = "These commands can be used: help, exit, commandlist";
        return true;
    }

    bool storeFile(rapidjson::Value& payload)
    {
        // nobinary file (when content could require base64 encoding/decoding)
        std::cout << "Controller::storeFile: command: \n";
        if (!payload.HasMember("fileName") || !payload.HasMember("fileContent"))
            return false;

        auto & fileNameValue = payload["fileName"];
        auto & fileNameContent = payload["fileContent"];
        if (! fileNameValue.IsString() || ! fileNameContent.IsString())
            return false;

        std::ofstream out(fileNameValue.GetString());
        out << fileNameContent.GetString();
        out.close();
        return true;
    }
};

// we will define this datatype because when we decide to change some parameters we can do it here and not in all function declaration (type is too complex) 
// but I would prefer hide it in CommandDispatcher class because it's related only to dispatcher
typedef std::function<bool(rapidjson::Value &)> CommandHandler;

class CommandDispatcher 
{
public:
    CommandDispatcher() = default; 
    // virtual destructor is not necessary - use virtual when someone will inherit from this class | compiler will create the destructor for me

    void addCommandHandler(std::string command, CommandHandler handler)
    {
        std::cout << "CommandDispatcher: addCommandHandler: " << command << std::endl;
        command_handlers_.insert(std::make_pair(command, handler));
    }

    bool dispatchCommand(std::string command_json)
    {
        std::cout << "COMMAND: " << command_json << std::endl;

        rapidjson::Document document;
        if (!parseCommand(document, command_json))
            return false;

        std::string command = document["command"].GetString();
        auto it = command_handlers_.find(command);
        if (it == command_handlers_.end())
            return false;

        auto & commandValue = document["payload"];
        if (! it->second(commandValue))
            return false;
        
        // print the commandValue because this is in/out parameter and some output can be added
        printOutValue(commandValue);
        return true;
    }

private:
    void printOutValue(rapidjson::Value& payload)
    {
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        payload.Accept(writer);
        std::cout << buffer.GetString() << std::endl;
    }

    bool parseCommand(rapidjson::Document & document, const std::string & command_json) const
    {
        const static std::string jsonSchema {R"(
        {   
            "type": "object",
		    "properties": 
            {
			    "command": { "type": "string" },
			    "payload": 
                {
				    "type": "object",
				    "properties": {}
                },
				"required": [ "command", "payload" ]
			}
        }
        )"};

        if (document.Parse(command_json.c_str()).HasParseError())
            return false;

        rapidjson::Document documentSchema;
        if (documentSchema.Parse(jsonSchema.c_str()).HasParseError())
            return false;

        rapidjson::SchemaDocument schemaDocument(documentSchema);
        rapidjson::SchemaValidator validator(schemaDocument);
        return document.Accept(validator);
    }

private:
    std::map<std::string, CommandHandler> command_handlers_;

    // we want to make dispatcher class "move only", so we remove copy constructor and assignment operator 
    CommandDispatcher (const CommandDispatcher&) = delete;
    CommandDispatcher& operator= (const CommandDispatcher&) = delete;
};

int main()
{
    std::cout << "COMMAND DISPATCHER: STARTED" << std::endl;

    CommandDispatcher command_dispatcher;
    Controller controller;

    // lambda alternative
    // command_dispatcher.addCommandHandler("help", std::move([&controller](rapidjson::Value& jsonValue) { return controller.help(jsonValue); }));

    command_dispatcher.addCommandHandler("help", std::bind(&Controller::help, &controller, std::placeholders::_1));
    command_dispatcher.addCommandHandler("exit", std::bind(&Controller::exit, &controller, std::placeholders::_1));
    command_dispatcher.addCommandHandler("commandList", std::bind(&Controller::commandList, &controller, std::placeholders::_1));
    command_dispatcher.addCommandHandler("storeFile", std::bind(&Controller::storeFile, &controller, std::placeholders::_1));

    // command line interface
    std::string command;
    while( ! g_done ) 
    {
        std::cout << "COMMANDS: {\"command\":\"exit\", \"payload\":{\"reason\":\"User requested exit.\"}}\n";
        std::cout << "\tenter command : ";
        std::getline(std::cin, command);
        command_dispatcher.dispatchCommand(command);
    }

    std::cout << "COMMAND DISPATCHER: ENDED" << std::endl;
    return 0;
}