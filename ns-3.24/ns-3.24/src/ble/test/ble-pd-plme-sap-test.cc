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
 * Author: Gary Pei <guangyu.pei@boeing.com>
 */
#include <ns3/log.h>
#include <ns3/test.h>
#include <ns3/packet.h>
#include <ns3/ble-phy.h>
#include <ns3/ble-mac.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>

using namespace ns3;

// This is an example TestCase.
class BlePlmeAndPdInterfaceTestCase : public TestCase
{
public:
  BlePlmeAndPdInterfaceTestCase ();
  virtual ~BlePlmeAndPdInterfaceTestCase ();

private:
  virtual void DoRun (void);
  void ReceivePdDataInndication (uint32_t psduLength, Ptr<Packet> p, uint8_t lqi);
};

BlePlmeAndPdInterfaceTestCase::BlePlmeAndPdInterfaceTestCase ()
  : TestCase ("Test the PLME and PD SAP per IEEE 802.15.4")
{
}

BlePlmeAndPdInterfaceTestCase::~BlePlmeAndPdInterfaceTestCase ()
{
}

void
BlePlmeAndPdInterfaceTestCase::ReceivePdDataInndication (uint32_t psduLength,
                                                            Ptr<Packet> p,
                                                            uint8_t lqi)
{
  NS_LOG_UNCOND ("At: " << Simulator::Now ()
                        << " Received frame size: " << psduLength << " LQI: " <<
                 lqi);
}

void
BlePlmeAndPdInterfaceTestCase::DoRun (void)
{
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnable ("BlePhy", LOG_LEVEL_ALL);

  Ptr<BlePhy> sender = CreateObject<BlePhy> ();
  Ptr<BlePhy> receiver = CreateObject<BlePhy> ();

  Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel> ();
  sender->SetChannel (channel);
  receiver->SetChannel (channel);

  receiver->SetPdDataIndicationCallback (MakeCallback (
                                           &BlePlmeAndPdInterfaceTestCase::ReceivePdDataInndication,
                                           this));

  uint32_t n = 10;
  Ptr<Packet> p = Create<Packet> (n);
  sender->PdDataRequest (p->GetSize (), p);

  Simulator::Destroy ();
}

// ==============================================================================
class BlePlmeAndPdInterfaceTestSuite : public TestSuite
{
public:
  BlePlmeAndPdInterfaceTestSuite ();
};

BlePlmeAndPdInterfaceTestSuite::BlePlmeAndPdInterfaceTestSuite ()
  : TestSuite ("ble-plme-pd-sap", UNIT)
{
  AddTestCase (new BlePlmeAndPdInterfaceTestCase, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static BlePlmeAndPdInterfaceTestSuite blePlmeAndPdInterfaceTestSuite;
