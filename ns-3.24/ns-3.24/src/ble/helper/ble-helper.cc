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
 * Authors:
 *  Gary Pei <guangyu.pei@boeing.com>
 *  Tom Henderson <thomas.r.henderson@boeing.com>
 */
#include "ble-helper.h"
#include <ns3/ble-error-model.h>
#include <ns3/ble-net-device.h>
#include <ns3/mobility-model.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/log.h>
#include "ns3/names.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BleHelper");

/**
 * @brief Output an ascii line representing the Transmit event (with context)
 * @param stream the output stream
 * @param context the context
 * @param p the packet
 */
static void
AsciiBleMacTransmitSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ptr<const Packet> p)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << context << " " << *p << std::endl;
}

/**
 * @brief Output an ascii line representing the Transmit event (without context)
 * @param stream the output stream
 * @param p the packet
 */
static void
AsciiBleMacTransmitSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  Ptr<const Packet> p)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << *p << std::endl;
}

BleHelper::BleHelper (void)
{
  m_channel = CreateObject<SingleModelSpectrumChannel> ();

  Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  m_channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  m_channel->SetPropagationDelayModel (delayModel);
}

BleHelper::BleHelper (bool useMultiModelSpectrumChannel)
{
  if (useMultiModelSpectrumChannel)
    {
      m_channel = CreateObject<MultiModelSpectrumChannel> ();
    }
  else
    {
      m_channel = CreateObject<SingleModelSpectrumChannel> ();
    }
  Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  m_channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  m_channel->SetPropagationDelayModel (delayModel);
}

BleHelper::~BleHelper (void)
{
  m_channel->Dispose ();
  m_channel = 0;
}

void
BleHelper::EnableLogComponents (void)
{
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnable ("BleErrorModel", LOG_LEVEL_ALL);
  LogComponentEnable ("BleInterferenceHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("BleMac", LOG_LEVEL_ALL);
  LogComponentEnable ("BleNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("BlePhy", LOG_LEVEL_ALL);
  LogComponentEnable ("BleSpectrumSignalParameters", LOG_LEVEL_ALL);
  LogComponentEnable ("BleSpectrumValueHelper", LOG_LEVEL_ALL);
}

std::string
BleHelper::BlePhyEnumerationPrinter (BlePhyEnumeration e)
{
  switch (e)
    {
    case IEEE_802_15_4_PHY_BUSY:
      return std::string ("BUSY");
    case IEEE_802_15_4_PHY_BUSY_RX:
      return std::string ("BUSY_RX");
    case IEEE_802_15_4_PHY_BUSY_TX:
      return std::string ("BUSY_TX");
    case IEEE_802_15_4_PHY_FORCE_TRX_OFF:
      return std::string ("FORCE_TRX_OFF");
    case IEEE_802_15_4_PHY_IDLE:
      return std::string ("IDLE");
    case IEEE_802_15_4_PHY_INVALID_PARAMETER:
      return std::string ("INVALID_PARAMETER");
    case IEEE_802_15_4_PHY_RX_ON:
      return std::string ("RX_ON");
    case IEEE_802_15_4_PHY_SUCCESS:
      return std::string ("SUCCESS");
    case IEEE_802_15_4_PHY_TRX_OFF:
      return std::string ("TRX_OFF");
    case IEEE_802_15_4_PHY_TX_ON:
      return std::string ("TX_ON");
    case IEEE_802_15_4_PHY_UNSUPPORTED_ATTRIBUTE:
      return std::string ("UNSUPPORTED_ATTRIBUTE");
    case IEEE_802_15_4_PHY_READ_ONLY:
      return std::string ("READ_ONLY");
    case IEEE_802_15_4_PHY_UNSPECIFIED:
      return std::string ("UNSPECIFIED");
    default:
      return std::string ("INVALID");
    }
}

std::string
BleHelper::BleMacStatePrinter (BleMacState e)
{
  switch (e)
    {
    case MAC_IDLE:
      return std::string ("MAC_IDLE");
    case CHANNEL_ACCESS_FAILURE:
      return std::string ("CHANNEL_ACCESS_FAILURE");
    case CHANNEL_IDLE:
      return std::string ("CHANNEL_IDLE");
    case SET_PHY_TX_ON:
      return std::string ("SET_PHY_TX_ON");
    default:
      return std::string ("INVALID");
    }
}

void
BleHelper::AddMobility (Ptr<BlePhy> phy, Ptr<MobilityModel> m)
{
  phy->SetMobility (m);
}

NetDeviceContainer
BleHelper::Install (NodeContainer c)
{
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<Node> node = *i;

      Ptr<BleNetDevice> netDevice = CreateObject<BleNetDevice> ();
      netDevice->SetChannel (m_channel);
      node->AddDevice (netDevice);
      netDevice->SetNode (node);
      // \todo add the capability to change short address, extended
      // address and panId. Right now they are hardcoded in BleMac::BleMac ()
      devices.Add (netDevice);
    }
  return devices;
}


Ptr<SpectrumChannel>
BleHelper::GetChannel (void)
{
  return m_channel;
}

void
BleHelper::SetChannel (Ptr<SpectrumChannel> channel)
{
  m_channel = channel;
}

void
BleHelper::SetChannel (std::string channelName)
{
  Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel> (channelName);
  m_channel = channel;
}


int64_t
BleHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<NetDevice> netDevice;
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<BleNetDevice> ble = DynamicCast<BleNetDevice> (netDevice);
      if (ble)
        {
          currentStream += ble->AssignStreams (currentStream);
        }
    }
  return (currentStream - stream);
}

void
BleHelper::AssociateToPan (NetDeviceContainer c, uint16_t panId)
{
  NetDeviceContainer devices;
  uint16_t id = 1;
  uint8_t idBuf[2];

  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<BleNetDevice> device = DynamicCast<BleNetDevice> (*i);
      if (device)
        {
          idBuf[0] = (id >> 8) & 0xff;
          idBuf[1] = (id >> 0) & 0xff;
          Mac16Address address;
          address.CopyFrom (idBuf);

          device->GetMac ()->SetPanId (panId);
          device->GetMac ()->SetShortAddress (address);
          id++;
        }
    }
  return;
}

/**
 * @brief Write a packet in a PCAP file
 * @param file the output file
 * @param packet the packet
 */
static void
PcapSniffBle (Ptr<PcapFileWrapper> file, Ptr<const Packet> packet)
{
  file->Write (Simulator::Now (), packet);
}

void
BleHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  NS_LOG_FUNCTION (this << prefix << nd << promiscuous << explicitFilename);
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.
  //

  // In the future, if we create different NetDevice types, we will
  // have to switch on each type below and insert into the right
  // NetDevice type
  //
  Ptr<BleNetDevice> device = nd->GetObject<BleNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("BleHelper::EnablePcapInternal(): Device " << device << " not of type ns3::BleNetDevice");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out,
                                                     PcapHelper::DLT_IEEE802_15_4);

  if (promiscuous == true)
    {
      device->GetMac ()->TraceConnectWithoutContext ("PromiscSniffer", MakeBoundCallback (&PcapSniffBle, file));

    }
  else
    {
      device->GetMac ()->TraceConnectWithoutContext ("Sniffer", MakeBoundCallback (&PcapSniffBle, file));
    }
}

void
BleHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream,
  std::string prefix,
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  Ptr<BleNetDevice> device = nd->GetObject<BleNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("BleHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::BleNetDevice");
      return;
    }

  //
  // Our default trace sinks are going to use packet printing, so we have to
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create
  // one using the usual trace filename conventions and do a Hook*WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      // Ascii traces typically have "+", '-", "d", "r", and sometimes "t"
      // The Mac and Phy objects have the trace sources for these
      //

      asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<BleNetDevice> (device, "MacRx", theStream);

      device->GetMac ()->TraceConnectWithoutContext ("MacTx", MakeBoundCallback (&AsciiBleMacTransmitSinkWithoutContext, theStream));

      asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<BleNetDevice> (device, "MacTxEnqueue", theStream);
      asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<BleNetDevice> (device, "MacTxDequeue", theStream);
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<BleNetDevice> (device, "MacTxDrop", theStream);

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to provide a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // Note that we are going to use the default trace sinks provided by the
  // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
  // but the default trace sinks are actually publicly available static
  // functions that are always there waiting for just such a case.
  //


  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::BleNetDevice/Mac/MacRx";
  device->GetMac ()->TraceConnect ("MacRx", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::BleNetDevice/Mac/MacTx";
  device->GetMac ()->TraceConnect ("MacTx", oss.str (), MakeBoundCallback (&AsciiBleMacTransmitSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::BleNetDevice/Mac/MacTxEnqueue";
  device->GetMac ()->TraceConnect ("MacTxEnqueue", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::BleNetDevice/Mac/MacTxDequeue";
  device->GetMac ()->TraceConnect ("MacTxDequeue", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::BleNetDevice/Mac/MacTxDrop";
  device->GetMac ()->TraceConnect ("MacTxDrop", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

}

} // namespace ns3

