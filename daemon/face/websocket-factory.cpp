/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  Regents of the University of California,
 *                     Arizona Board of Regents,
 *                     Colorado State University,
 *                     University Pierre & Marie Curie, Sorbonne University,
 *                     Washington University in St. Louis,
 *                     Beijing Institute of Technology,
 *                     The University of Memphis
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "websocket-factory.hpp"

namespace nfd {

using namespace boost::asio;

NFD_LOG_INIT("WebSocketFactory");

WebSocketFactory::WebSocketFactory(const std::string& defaultPort)
  : m_defaultPort(defaultPort)
{
}

shared_ptr<WebSocketChannel>
WebSocketFactory::createChannel(const websocket::Endpoint& endpoint)
{
  shared_ptr<WebSocketChannel> channel = findChannel(endpoint);
  if (static_cast<bool>(channel))
    return channel;

  channel = make_shared<WebSocketChannel>(boost::cref(endpoint));
  m_channels[endpoint] = channel;

  return channel;
}

shared_ptr<WebSocketChannel>
WebSocketFactory::createChannel(const std::string& localIPAddress,
                                uint16_t localPort)
{
  boost::system::error_code ec;
  ip::address address = ip::address::from_string(localIPAddress, ec);
  if (ec)
    {
      throw Error("Invalid address format: " + localIPAddress);
    }
  websocket::Endpoint endpoint(address, localPort);
  return createChannel(endpoint);
}

shared_ptr<WebSocketChannel>
WebSocketFactory::findChannel(const websocket::Endpoint& localEndpoint)
{
  ChannelMap::iterator i = m_channels.find(localEndpoint);
  if (i != m_channels.end())
    return i->second;
  else
    return shared_ptr<WebSocketChannel>();
}

void
WebSocketFactory::createFace(const FaceUri& uri,
                             const FaceCreatedCallback& onCreated,
                             const FaceConnectFailedCallback& onConnectFailed)
{
  throw Error("WebSocketFactory does not support 'createFace' operation");
}

} // namespace nfd
