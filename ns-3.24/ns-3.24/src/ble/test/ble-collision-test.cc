/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Universita' di Firenze, Italy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#include <ns3/test.h>
#include <ns3/packet.h>
#include <ns3/ble-module.h>
#include <ns3/mobility-module.h>
#include <ns3/propagation-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>
#include <ns3/log.h>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ble-collision-test");

// This is an example TestCase.
class BleCollisionTestCase : public TestCase
{
public:
  BleCollisionTestCase ();
  virtual ~BleCollisionTestCase ();

  void DataIndication (McpsDataIndicationParams params, Ptr<Packet> p);
private:
  virtual void DoRun (void);

  uint8_t m_rxPackets;
};

BleCollisionTestCase::BleCollisionTestCase ()
  : TestCase ("Test the 802.15.4 collision handling")
{
  m_rxPackets = 0;
}

BleCollisionTestCase::~BleCollisionTestCase ()
{
}

void BleCollisionTestCase::DataIndication (McpsDataIndicationParams params, Ptr<Packet> p)
{
  m_rxPackets++;
}


void
BleCollisionTestCase::DoRun (void)
{

  // Create 3 nodes, and a NetDevice for each one
  Ptr<Node> n0 = CreateObject <Node> ();
  Ptr<Node> n1 = CreateObject <Node> ();
  Ptr<Node> n2 = CreateObject <Node> ();

  Ptr<BleNetDevice> dev0 = CreateObject<BleNetDevice> ();
  Ptr<BleNetDevice> dev1 = CreateObject<BleNetDevice> ();
  Ptr<BleNetDevice> dev2 = CreateObject<BleNetDevice> ();

  dev0->SetAddress (Mac16Address ("00:01"));
  dev1->SetAddress (Mac16Address ("00:02"));
  dev2->SetAddress (Mac16Address ("00:03"));

  // Each device must be attached to the same channel
  Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel> ();
  Ptr<LogDistancePropagationLossModel> propModel = CreateObject<LogDistancePropagationLossModel> ();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->AddPropagationLossModel (propModel);
  channel->SetPropagationDelayModel (delayModel);

  dev0->SetChannel (channel);
  dev1->SetChannel (channel);
  dev2->SetChannel (channel);

  // To complete configuration, a BleNetDevice must be added to a node
  n0->AddDevice (dev0);
  n1->AddDevice (dev1);
  n2->AddDevice (dev2);

  Ptr<ConstantPositionMobilityModel> sender0Mobility = CreateObject<ConstantPositionMobilityModel> ();
  sender0Mobility->SetPosition (Vector (0,0,0));
  dev0->GetPhy ()->SetMobility (sender0Mobility);
  n0->AggregateObject (sender0Mobility);

  Ptr<ConstantPositionMobilityModel> sender1Mobility = CreateObject<ConstantPositionMobilityModel> ();
  // Configure position 10 m distance
  sender1Mobility->SetPosition (Vector (0,1,0));
  dev1->GetPhy ()->SetMobility (sender1Mobility);
  n1->AggregateObject (sender1Mobility);

  Ptr<ConstantPositionMobilityModel> sender2Mobility = CreateObject<ConstantPositionMobilityModel> ();
  // Configure position 10 m distance
  sender2Mobility->SetPosition (Vector (30,0,0));
  dev2->GetPhy ()->SetMobility (sender2Mobility);
  n2->AggregateObject (sender2Mobility);

  dev0->GetMac ()->SetMcpsDataIndicationCallback (MakeCallback (&BleCollisionTestCase::DataIndication, this));

  // Disable first backoff
  dev0->GetCsmaCa ()->SetMacMinBE (0);
  dev1->GetCsmaCa ()->SetMacMinBE (0);
  dev2->GetCsmaCa ()->SetMacMinBE (0);

  Ptr<Packet> p0 = Create<Packet> (20);
  Ptr<Packet> p1 = Create<Packet> (60);
  Ptr<Packet> p2 = Create<Packet> (100);

  McpsDataRequestParams params;
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstPanId = 0;
  params.m_msduHandle = 0;
  // params.m_txOptions = TX_OPTION_ACK;

  // First case: concurrent tx and no ACKs
  std::cout << "*** First test " << std::endl;
  m_rxPackets = 0;
  params.m_dstAddr = Mac16Address ("00:02");
  Simulator::Schedule (Seconds (0.1),
                       &BleMac::McpsDataRequest,
                       dev0->GetMac (), params, p0);

  params.m_dstAddr = Mac16Address ("00:01");
  Simulator::Schedule (Seconds (0.1),
                       &BleMac::McpsDataRequest,
                       dev1->GetMac (), params, p1);

  Simulator::Run ();

  NS_TEST_EXPECT_MSG_EQ (m_rxPackets, 0, "Not received a packet (as expected)");

  // Second case: concurrent tx and ACKs
  std::cout << "*** Second test " << std::endl;
  m_rxPackets = 0;
  params.m_txOptions = TX_OPTION_ACK;

  params.m_dstAddr = Mac16Address ("00:02");
  Simulator::Schedule (Seconds (0.1),
                       &BleMac::McpsDataRequest,
                       dev0->GetMac (), params, p0);

  params.m_dstAddr = Mac16Address ("00:01");
  Simulator::Schedule (Seconds (0.1),
                       &BleMac::McpsDataRequest,
                       dev1->GetMac (), params, p1);

  Simulator::Run ();

  NS_TEST_EXPECT_MSG_EQ (m_rxPackets, 1, "Received a packet (as expected)");

  // Third case: two concurrent tx and no ACKs
  std::cout << "*** Third test " << std::endl;
  m_rxPackets = 0;
  params.m_txOptions = 0;

//  LogComponentEnable("BleMac",LOG_LEVEL_ALL);
//  LogComponentEnable("BlePhy",LOG_LEVEL_ALL);
//  LogComponentEnableAll (LOG_PREFIX_TIME);

  params.m_dstAddr = Mac16Address ("00:01");
  Simulator::Schedule (Seconds (0.0001),
                       &BleMac::McpsDataRequest,
                       dev2->GetMac (), params, p2);

  params.m_dstAddr = Mac16Address ("00:01");
  Simulator::Schedule (Seconds (0.0002),
                       &BleMac::McpsDataRequest,
                       dev1->GetMac (), params, p0);

  Simulator::Run ();

  std::cout << "m_rxPackets = " << int(m_rxPackets) << std::endl;
  NS_TEST_EXPECT_MSG_EQ (m_rxPackets, 1, "Received a packet (as expected)");

  // Fourth case: two concurrent tx and ACKs
  std::cout << "*** Fourth test " << std::endl;
  m_rxPackets = 0;
  params.m_txOptions = TX_OPTION_ACK;

  params.m_dstAddr = Mac16Address ("00:01");
  Simulator::Schedule (Seconds (0.1),
                       &BleMac::McpsDataRequest,
                       dev1->GetMac (), params, p0);

  params.m_dstAddr = Mac16Address ("00:01");
  Simulator::Schedule (Seconds (0.1),
                       &BleMac::McpsDataRequest,
                       dev2->GetMac (), params, p1);

  Simulator::Run ();

  std::cout << "m_rxPackets = " << int(m_rxPackets) << std::endl;
  NS_TEST_EXPECT_MSG_EQ (m_rxPackets, 2, "Received two packets (as expected)");

  Simulator::Destroy ();
}

// ==============================================================================
class BleCollisionTestSuite : public TestSuite
{
public:
  BleCollisionTestSuite ();
};

BleCollisionTestSuite::BleCollisionTestSuite ()
  : TestSuite ("ble-collision", UNIT)
{
  AddTestCase (new BleCollisionTestCase, TestCase::QUICK);
}

static BleCollisionTestSuite g_bleCollisionTestSuite;
