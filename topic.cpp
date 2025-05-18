// void Server::parseTopicCommand( int userFd, const std::string& command)
// {
//     std::vector<std::string> tokens;
//     std::string::size_type   start = 0;
//     std::string::size_type   end = command.find(' ');

//     // Skip any leading spaces after the command name
//     while (end != std::string::npos && command[start] == ' ')
//     {
//         ++start;
//         end = command.find(' ', start);
//     }

//     if (end == std::string::npos)
//         return;

//     tokens.push_back(command.substr(start, end - start)); // Command name (e.g., "TOPIC")
//     start = end + 1;

//     // Skip additional spaces
//     while (start < command.length() && command[start] == ' ')
//         ++start;

//     if (start >= command.length())
//     {
//         // Missing channel parameter
//         sendError(userFd, "461", "TOPIC", "Not enough parameters");
//         return;
//     }

//     // Extract channel
//     end = command.find(' ', start);
//     std::string channel = command.substr(start, end - start);
//     start = (end == std::string::npos) ? std::string::npos : end + 1;

//     // Skip additional spaces before topic (if present)
//     while (start != std::string::npos && start < command.length() && command[start] == ' ')
//         ++start;

//     // Extract topic if provided (including colon)
//     std::string topic;
//     if (start != std::string::npos && start < command.length())
//     {
//         if (command[start] == ':')
//             ++start; // Skip colon
//         topic = command.substr(start);
//     }

//     // Pass the extracted channel and topic (possibly empty) to the handler
//     topicCommand(userFd, channel, topic);
// }