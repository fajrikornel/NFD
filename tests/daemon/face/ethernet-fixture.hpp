/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
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
 */

#ifndef NFD_TESTS_DAEMON_FACE_ETHERNET_FIXTURE_HPP
#define NFD_TESTS_DAEMON_FACE_ETHERNET_FIXTURE_HPP

#include "core/network-interface.hpp"
#include "face/multicast-ethernet-transport.hpp"
#include "face/unicast-ethernet-transport.hpp"

#include "tests/limited-io.hpp"

namespace nfd {
namespace face {
namespace tests {

using namespace nfd::tests;

class EthernetFixture : public virtual BaseFixture
{
protected:
  EthernetFixture()
  {
    for (const auto& netif : listNetworkInterfaces()) {
      if (!netif.isLoopback() && netif.isUp()) {
        try {
          MulticastEthernetTransport transport(*netif.asNetworkInterface(),
                                               ethernet::getBroadcastAddress(),
                                               ndn::nfd::LINK_TYPE_MULTI_ACCESS);
          netifs.push_back(netif);
        }
        catch (const EthernetTransport::Error&) {
          // ignore
        }
      }
    }
  }

  void
  initializeUnicast(ndn::nfd::FacePersistency persistency = ndn::nfd::FACE_PERSISTENCY_PERSISTENT,
                    ethernet::Address remoteAddr = {0x00, 0x00, 0x5e, 0x00, 0x53, 0x5e})
  {
    BOOST_ASSERT(netifs.size() > 0);
    localEp = netifs.front().name;
    remoteEp = remoteAddr;
    transport = make_unique<UnicastEthernetTransport>(*netifs.front().asNetworkInterface(),
                                                      remoteEp, persistency, time::seconds(2));
  }

  void
  initializeMulticast(ndn::nfd::LinkType linkType = ndn::nfd::LINK_TYPE_MULTI_ACCESS,
                      ethernet::Address mcastGroup = {0x01, 0x00, 0x5e, 0x90, 0x10, 0x5e})
  {
    BOOST_ASSERT(netifs.size() > 0);
    localEp = netifs.front().name;
    remoteEp = mcastGroup;
    transport = make_unique<MulticastEthernetTransport>(*netifs.front().asNetworkInterface(),
                                                        remoteEp, linkType);
  }

protected:
  LimitedIo limitedIo;
  unique_ptr<EthernetTransport> transport;
  std::string localEp;
  ethernet::Address remoteEp;

  /** \brief EthernetTransport-capable network interfaces
   */
  std::vector<NetworkInterfaceInfo> netifs;
};

#define SKIP_IF_ETHERNET_NETIF_COUNT_LT(n) \
  do { \
    if (this->netifs.size() < (n)) { \
      BOOST_WARN_MESSAGE(false, "skipping assertions that require " #n \
                                " or more EthernetTransport-capable network interfaces"); \
      return; \
    } \
  } while (false)

} // namespace tests
} // namespace face
} // namespace nfd

#endif // NFD_TESTS_DAEMON_FACE_ETHERNET_FIXTURE_HPP
