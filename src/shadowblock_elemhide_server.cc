// ShadowBlock project
// UCR Security Lab/UIowa IRL 2019
// @author Shitong Zhu
// https://github.com/seclab-ucr/ShadowBlock

#include "include/AdblockPlus.h"
#include "include/AdblockPlus/Platform.h"

#include <iostream>
#include <vector>
#include <numeric>
#include <set>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0
// You may change this, but need to make it consistent with
// the port used in Chromium
#define SERVER_PORT 8888
// Large enough to contain very long CSS selectors
#define SELECTOR_STRING_OVERALL_BUFFER_SIZE 1024000

using namespace AdblockPlus;

const std::string MergeAllSelectors(std::vector<std::string> vec)
{
  std::string res = "";
  for (int i = 0; i < vec.size() - 1; i++)
  {
    res.append(vec[i]);
    res.append(",");
  }
  res.append(vec[vec.size() - 1]);
  return res;
}

int main()
{
  AdblockPlus::AppInfo appInfo;
  appInfo.version = "1.0";
  appInfo.name = "elemhidingremoteshell";
  appInfo.application = "standalone";
  appInfo.applicationVersion = "1.0";
  appInfo.locale = "en-US";

  auto platform = DefaultPlatformBuilder().CreatePlatform();
  platform->SetUpJsEngine(appInfo);
  JsEngine &jsEngine = platform->GetJsEngine();
  auto &filterEngine = platform->GetFilterEngine();

  int opt = TRUE;
  int master_socket, addrlen, new_socket, client_socket[300],
      max_clients = 300, activity, i, valread, sd;
  int max_sd;
  struct sockaddr_in address;

  std::cout << "[SB][elemhide-server][info] Launching the HTML rule module of ShadowBlock\n"
            << std::endl;

  // Data buffer of 1K
  char buffer[SELECTOR_STRING_OVERALL_BUFFER_SIZE];

  // Set of socket descriptors
  fd_set readfds;

  // Initialize all client_socket[] to 0 so not checked
  for (i = 0; i < max_clients; i++)
  {
    client_socket[i] = 0;
  }

  // Create a master socket
  if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Set master socket to allow multiple connections ,
  if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                 sizeof(opt)) < 0)
  {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // Type of socket created
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(SERVER_PORT);

  // Bind the socket to localhost port 8888
  if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  printf("[SB][elemhide-server][info] Listening on port %d\n", SERVER_PORT);

  // try to specify maximum of 3 pending connections for the master socket
  if (listen(master_socket, 3) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  // accept the incoming connection
  addrlen = sizeof(address);
  puts("[SB][elemhide-server][info] Waiting for connections");

  while (TRUE)
  {
    // Clear the socket set
    FD_ZERO(&readfds);

    // Add master socket to set
    FD_SET(master_socket, &readfds);
    max_sd = master_socket;

    // Add child sockets to set
    for (i = 0; i < max_clients; i++)
    {
      // Socket descriptor
      sd = client_socket[i];

      // If it's valid socket descriptor then add to read list
      if (sd > 0)
        FD_SET(sd, &readfds);

      // Highest file descriptor number, need it for the select function
      if (sd > max_sd)
        max_sd = sd;
    }

    // Waiting for an activity on one of the sockets
    // Timeout being NULL means indefinitely
    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if ((activity < 0) && (errno != EINTR))
    {
      printf("[SB][elemhide-server][error] Select error");
    }

    // If something happened on the master socket,
    // then its an incoming connection
    if (FD_ISSET(master_socket, &readfds))
    {
      if ((new_socket = accept(master_socket, (struct sockaddr *)&address,
                               (socklen_t *)&addrlen)) < 0)
      {
        perror("accept");
        exit(EXIT_FAILURE);
      }

      // Inform user of socket number
      printf("[SB][elemhide-server][info] New incoming connection\n");

      // Add new socket to array of sockets
      for (i = 0; i < max_clients; i++)
      {
        // If position is empty
        if (client_socket[i] == 0)
        {
          client_socket[i] = new_socket;
          printf("[SB][elemhide-server][info] Adding to list of sockets\n");
          break;
        }
      }
    }

    // Else its some IO operation on some other socket
    for (i = 0; i < max_clients; i++)
    {
      sd = client_socket[i];

      if (FD_ISSET(sd, &readfds))
      {
        // Check if it was for closing, and also read the
        // incoming message
        if ((valread = read(sd, buffer, SELECTOR_STRING_OVERALL_BUFFER_SIZE)) <=
            0)
        {
          // One client is being disconnected
          getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
          printf("[SB][elemhide-server][info] Host disconnected, IP %s, PORT %d \n",
                 inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          // Close the socket and mark as 0 in list for reuse
          close(sd);
          client_socket[i] = 0;
        }

        // Echo back the message that came in
        else
        {
          std::cout << "[SB][elemhide-server][info] Length of msg: " << valread << "\n";

          buffer[valread] = '\0';
          std::vector<std::string> domain_selectors;
          std::vector<std::string> generic_selectors;

          std::string tab_url(buffer);
          std::cout << "[SB][elemhide-server][info] URL: " << tab_url << "\n";
          std::string host_domain = filterEngine.GetHostFromURL(tab_url);

          domain_selectors =
              filterEngine.GetElementHidingSelectors(host_domain);
          generic_selectors = filterEngine.GetElementHidingSelectors("");

          std::cout << "[SB][elemhide-server][info] Size of domain selectors: "
                    << domain_selectors.size()
                    << std::endl;
          std::cout << "[SB][elemhide-server][info] Size of generic selectors: "
                    << generic_selectors.size()
                    << std::endl;

          std::set<std::string> domain_selectors_set(domain_selectors.begin(),
                                                     domain_selectors.end());
          std::set<std::string> generic_selectors_set(generic_selectors.begin(),
                                                      generic_selectors.end());
          generic_selectors_set.insert(domain_selectors_set.begin(),
                                       domain_selectors_set.end());
          std::vector<std::string> selectors_final(generic_selectors_set.begin(),
                                                   generic_selectors_set.end());
          std::cout << "[SB][elemhide-server][info] Size of final selectors: "
                    << selectors_final.size()
                    << std::endl;
          std::string res_str = MergeAllSelectors(selectors_final);
          const char *res_c_str = res_str.c_str();

          send(sd, res_c_str, strlen(res_c_str), 0);
          std::cout << "[SB][elemhide-server][info] Selector string sent" << std::endl;
          const char *signal_to_shutdown = ",.NOTVERYUNIQUESTRING";
          send(sd, signal_to_shutdown, strlen(signal_to_shutdown), 0);
          std::cout << "[SB][elemhide-server][info] Stop signal sent\n"
                    << std::endl;
          close(sd);
          client_socket[i] = 0;
        }
      }
    }
  }

  return 0;
}
