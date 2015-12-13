/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Tom Henderson <thomas.r.henderson@boeing.com>
 */

// This program produces a gnuplot file that plots the packet success rate
// as a function of distance for the 802.15.4 models, assuming a default
// LogDistance propagation loss model, the 2.4 GHz OQPSK error model, a
// default transmit power of 0 dBm, and a default packet size of 20 bytes of
// 802.15.4 payload.
#include <ns3/test.h>
#include <ns3/log.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>
#include <ns3/ble-error-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/ble-net-device.h>
#include <ns3/spectrum-value.h>
#include <ns3/ble-spectrum-value-helper.h>
#include <ns3/ble-mac.h>
#include <ns3/node.h>
#include <ns3/net-device.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/mac16-address.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/uinteger.h>
#include <ns3/nstime.h>
#include <ns3/abort.h>
#include <ns3/command-line.h>
#include <ns3/gnuplot.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace ns3;
using namespace std;

static uint32_t g_received[400] = {};

NS_LOG_COMPONENT_DEFINE ("BleErrorDevicesPlot");

static void
BleErrorCallback (McpsDataIndicationParams params, Ptr<Packet> p)
{
  uint8_t *packet = (uint8_t *) malloc(sizeof(uint8_t) * 20);
  p->CopyData(packet, 20);
  uint32_t packetIndex = 0;
  packetIndex += 256 * packet[1];    
  packetIndex += packet[0];
  g_received[packetIndex]++;
  free(packet);
}

std::string 
decToHexStr(int dec)
{
  std::string hexStr;
  if (dec >= 0 && dec < 10) {
    hexStr = std::to_string(dec);  
  } else if (dec == 10) {
    hexStr = "a";  
  } else if (dec == 11) {
    hexStr = "b";  
  } else if (dec == 12) {
    hexStr = "c";  
  } else if (dec == 13) {
    hexStr = "d";  
  } else if (dec == 14) {
    hexStr = "e";  
  } else if (dec == 15) {
    hexStr = "f";  
  }
  
  return hexStr;
}

std::string
getMacAddressString(int macIntValue)
{
  std::string macString;

  for (int i = 3; i >= 0; i--) {
    macString += decToHexStr((int) pow(16, i));
    if (i == 2) {
      macString += ":";
    }
  }

  return macString;
}

Vector
baseTopologyVector(int index) 
{
  Vector v;
  int fixedDistance = 10;
  int normalizedIndex = index % 6;
  if (normalizedIndex == 0) {
    v = Vector(fixedDistance, 0, 0);
  } else if (normalizedIndex == 1) {
    v = Vector(-fixedDistance, 0, 0);
  } else if (normalizedIndex == 2) {
    v = Vector(0, fixedDistance, 0);
  } else if (normalizedIndex == 3) {
    v = Vector(0,-fixedDistance, 0);
  } else if (normalizedIndex == 4) {
    v = Vector(0, 0, fixedDistance);
  } else if (normalizedIndex == 5) {
    v = Vector(0,0, -fixedDistance);
  } 

  return v;
}

int main (int argc, char *argv[])
{
  std::ostringstream os;
  std::ofstream berfile ("802.15.4-psr-devices.plt");

  double intervalLength = 0.05; // length of beacon interval
  double numIntervals = 10;
  int deviceIncrement = 20;
  int packetSize = 20;
  uint8_t *packetBuffer;
  double txPower = 0;
  uint32_t channelNumber = 11;


  CommandLine cmd;

  cmd.AddValue ("txPower", "transmit power (dBm)", txPower);
  cmd.AddValue ("packetSize", "packet (MSDU) size (bytes)", packetSize);
  cmd.AddValue ("channelNumber", "channel number", channelNumber);

  cmd.Parse (argc, argv);

  os << "Packet (MSDU) size = " << packetSize << " bytes; tx power = " << txPower << " dBm; channel = " << channelNumber;

  Gnuplot psrplot = Gnuplot ("802.15.4-psr-distance.eps");
  Gnuplot2dDataset psrdataset ("802.15.4-psr-vs-distance");

  int devicesCount = 400;
  Ptr<Node> nodes [400];
  Ptr<BleNetDevice> netDevices [400];
  Ptr<ConstantPositionMobilityModel> positions[400];

  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<LogDistancePropagationLossModel> model = CreateObject<LogDistancePropagationLossModel> ();
  channel->AddPropagationLossModel (model);
  BleSpectrumValueHelper svh;
  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txPower, channelNumber);

  for (int i = 0; i < devicesCount; i++) {
    nodes[i] = CreateObject<Node> ();
    netDevices[i] = CreateObject<BleNetDevice> ();
    netDevices[i]->SetAddress (Mac16Address (getMacAddressString(i).c_str()));
    netDevices[i]->SetChannel (channel);
    nodes[i]->AddDevice (netDevices[i]);
    positions[i] = CreateObject<ConstantPositionMobilityModel> ();
    netDevices[i]->GetPhy ()->SetMobility(positions[i]);
    netDevices[i]->GetPhy ()->SetTxPowerSpectralDensity (psd);
  } 

  Ptr<Node> nodeReceiver = CreateObject <Node> ();
  Ptr<BleNetDevice> devReceiver = CreateObject<BleNetDevice> ();
  devReceiver->SetAddress (Mac16Address ("ee:ee"));
  devReceiver->SetChannel (channel);
  nodeReceiver->AddDevice (devReceiver);
  Ptr<ConstantPositionMobilityModel> mobReceiver = CreateObject<ConstantPositionMobilityModel> ();
  devReceiver->GetPhy ()->SetMobility (mobReceiver);

  McpsDataIndicationCallback cb0;
  cb0 = MakeCallback (&BleErrorCallback);
  devReceiver->GetMac ()->SetMcpsDataIndicationCallback (cb0);

  McpsDataRequestParams params;
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstPanId = 0;
  params.m_dstAddr = Mac16Address ("ff:ff");
  params.m_msduHandle = 0;
  params.m_txOptions = 0;

  Ptr<Packet> p;
  mobReceiver->SetPosition (Vector (0,0,0));

  // set the position of all nodes & initial delay offset
  for (int i = 0; i < devicesCount; i++) {
    // positions[i]->SetPosition(Vector(10, 0, 0));
    positions[i]->SetPosition(baseTopologyVector(i));
  }
  for (int k = 201; k <= devicesCount; k += deviceIncrement) {
    for (int j = 0; j < k;  j++)
    {
      double microSecDelay = rand() % (int) (1000000 * intervalLength);
      double initialDelay = microSecDelay / 1000000.0;
      packetBuffer = (uint8_t *) malloc(sizeof(uint8_t) * packetSize);
      packetBuffer[0] = j % 256;
      packetBuffer[1] = j / 256;

      for (int i = 0; i < numIntervals; i++)
        {
          p = Create<Packet> (packetBuffer, packetSize);
          Simulator::Schedule (Seconds (i * intervalLength + initialDelay),
                               &BleMac::McpsDataRequest,
                               netDevices[j]->GetMac (), params, p);
        }
        free(packetBuffer);
    }   

    Simulator::Run ();
    NS_LOG_DEBUG ("Ran simulation with " << k << " devices");
    int successfulDevices = 0;
    for (int i = 0; i < devicesCount; i++) {
      if (g_received[i] > 0) successfulDevices++;  
    }
    psrdataset.Add (k, successfulDevices / (double) k);    
  }

  psrplot.AddDataset (psrdataset);
  
  psrplot.SetTitle (os.str ());
  psrplot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
  psrplot.SetLegend ("distance (m)", "Packet Success Rate (PSR)");
  psrplot.SetExtra  ("set xrange [0:200]\n\
set yrange [0:1]\n\
set grid\n\
set style line 1 linewidth 5\n\
set style increment user");
  psrplot.GenerateOutput (berfile);
  berfile.close ();

  Simulator::Destroy ();
  return 0;
}

