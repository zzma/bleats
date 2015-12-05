/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 The Boeing Company
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
 * Author:  Tom Henderson <thomas.r.henderson@boeing.com>
 */
#include <ns3/test.h>
#include <ns3/packet.h>
#include <ns3/ble-mac-header.h>
#include <ns3/ble-mac-trailer.h>
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>
#include <ns3/log.h>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ble-packet-test");

// This is an example TestCase.
class BlePacketTestCase : public TestCase
{
public:
  BlePacketTestCase ();
  virtual ~BlePacketTestCase ();

private:
  virtual void DoRun (void);
};

BlePacketTestCase::BlePacketTestCase ()
  : TestCase ("Test the 802.15.4 MAC header and trailer classes")
{
}

BlePacketTestCase::~BlePacketTestCase ()
{
}

void
BlePacketTestCase::DoRun (void)
{

  BleMacHeader macHdr (BleMacHeader::BLE_MAC_BEACON, 0);        //sequence number set to 0
  macHdr.SetSrcAddrMode (BleMacHeader::SHORTADDR);                    // short addr
  macHdr.SetDstAddrMode (BleMacHeader::NOADDR);
  macHdr.SetSecDisable ();
  macHdr.SetNoPanIdComp ();
  // ... other setters

  uint16_t srcPanId = 100;
  Mac16Address srcBleAddr ("00:11");
  macHdr.SetSrcAddrFields (srcPanId, srcBleAddr);

  BleMacTrailer macTrailer;


  Ptr<Packet> p = Create<Packet> (20);  // 20 bytes of dummy data
  NS_TEST_ASSERT_MSG_EQ (p->GetSize (), 20, "Packet created with unexpected size");
  p->AddHeader (macHdr);
  std::cout << " <--Mac Header added " << std::endl;

  NS_TEST_ASSERT_MSG_EQ (p->GetSize (), 27, "Packet wrong size after macHdr addition");
  p->AddTrailer (macTrailer);
  NS_TEST_ASSERT_MSG_EQ (p->GetSize (), 29, "Packet wrong size after macTrailer addition");

  // Test serialization and deserialization
  uint32_t size = p->GetSerializedSize ();
  uint8_t buffer[size];
  p->Serialize (buffer, size);
  Ptr<Packet> p2 = Create<Packet> (buffer, size, true);


  p2->Print (std::cout);
  std::cout << " <--Packet P2 " << std::endl;

  NS_TEST_ASSERT_MSG_EQ (p2->GetSize (), 29, "Packet wrong size after deserialization");

  BleMacHeader receivedMacHdr;
  p2->RemoveHeader (receivedMacHdr);

  receivedMacHdr.Print (std::cout);
  std::cout << " <--P2 Mac Header " << std::endl;

  NS_TEST_ASSERT_MSG_EQ (p2->GetSize (), 22, "Packet wrong size after removing machdr");

  BleMacTrailer receivedMacTrailer;
  p2->RemoveTrailer (receivedMacTrailer);
  NS_TEST_ASSERT_MSG_EQ (p2->GetSize (), 20, "Packet wrong size after removing headers and trailers");
  // Compare macHdr with receivedMacHdr, macTrailer with receivedMacTrailer,...

}

// ==============================================================================
class BlePacketTestSuite : public TestSuite
{
public:
  BlePacketTestSuite ();
};

BlePacketTestSuite::BlePacketTestSuite ()
  : TestSuite ("ble-packet", UNIT)
{
  AddTestCase (new BlePacketTestCase, TestCase::QUICK);
}

static BlePacketTestSuite blePacketTestSuite;
