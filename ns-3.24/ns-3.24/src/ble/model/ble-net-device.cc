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
 * Author:
 *  Tom Henderson <thomas.r.henderson@boeing.com>
 *  Tommaso Pecorella <tommaso.pecorella@unifi.it>
 *  Margherita Filippetti <morag87@gmail.com>
 */
#include "ble-net-device.h"
#include "ble-phy.h"
#include "ble-error-model.h"
#include <ns3/abort.h>
#include <ns3/node.h>
#include <ns3/log.h>
#include <ns3/spectrum-channel.h>
#include <ns3/pointer.h>
#include <ns3/boolean.h>
#include <ns3/mobility-model.h>
#include <ns3/packet.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BleNetDevice");

NS_OBJECT_ENSURE_REGISTERED (BleNetDevice);

TypeId
BleNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BleNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName ("Ble")
    .AddConstructor<BleNetDevice> ()
    .AddAttribute ("Channel", "The channel attached to this device",
                   PointerValue (),
                   MakePointerAccessor (&BleNetDevice::DoGetChannel),
                   MakePointerChecker<SpectrumChannel> ())
    .AddAttribute ("Phy", "The PHY layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&BleNetDevice::GetPhy,
                                        &BleNetDevice::SetPhy),
                   MakePointerChecker<BlePhy> ())
    .AddAttribute ("Mac", "The MAC layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&BleNetDevice::GetMac,
                                        &BleNetDevice::SetMac),
                   MakePointerChecker<BleMac> ())
    .AddAttribute ("UseAcks", "Request acknowledgments for data frames.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&BleNetDevice::m_useAcks),
                   MakeBooleanChecker ())
  ;
  return tid;
}

BleNetDevice::BleNetDevice ()
  : m_configComplete (false)
{
  NS_LOG_FUNCTION (this);
  m_mac = CreateObject<BleMac> ();
  m_phy = CreateObject<BlePhy> ();
  CompleteConfig ();
}

BleNetDevice::~BleNetDevice ()
{
  NS_LOG_FUNCTION (this);
}


void
BleNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_mac->Dispose ();
  m_phy->Dispose ();
  m_phy = 0;
  m_mac = 0;
  m_node = 0;
  // chain up.
  NetDevice::DoDispose ();

}

void
BleNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_phy->Initialize ();
  m_mac->Initialize ();
  NetDevice::DoInitialize ();
}


void
BleNetDevice::CompleteConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (m_mac == 0
      || m_phy == 0
      || m_node == 0
      || m_configComplete)
    {
      return;
    }
  m_mac->SetPhy (m_phy);
  m_mac->SetMcpsDataIndicationCallback (MakeCallback (&BleNetDevice::McpsDataIndication, this));

  Ptr<MobilityModel> mobility = m_node->GetObject<MobilityModel> ();
  if (!mobility)
    {
      NS_LOG_WARN ("BleNetDevice: no Mobility found on the node, probably it's not a good idea.");
    }
  m_phy->SetMobility (mobility);
  Ptr<BleErrorModel> model = CreateObject<BleErrorModel> ();
  m_phy->SetErrorModel (model);
  m_phy->SetDevice (this);

  m_phy->SetPdDataIndicationCallback (MakeCallback (&BleMac::PdDataIndication, m_mac));
  m_phy->SetPdDataConfirmCallback (MakeCallback (&BleMac::PdDataConfirm, m_mac));
  m_phy->SetPlmeEdConfirmCallback (MakeCallback (&BleMac::PlmeEdConfirm, m_mac));
  m_phy->SetPlmeGetAttributeConfirmCallback (MakeCallback (&BleMac::PlmeGetAttributeConfirm, m_mac));
  m_phy->SetPlmeSetTRXStateConfirmCallback (MakeCallback (&BleMac::PlmeSetTRXStateConfirm, m_mac));
  m_phy->SetPlmeSetAttributeConfirmCallback (MakeCallback (&BleMac::PlmeSetAttributeConfirm, m_mac));

  m_configComplete = true;
}

void
BleNetDevice::SetMac (Ptr<BleMac> mac)
{
  NS_LOG_FUNCTION (this);
  m_mac = mac;
  CompleteConfig ();
}

void
BleNetDevice::SetPhy (Ptr<BlePhy> phy)
{
  NS_LOG_FUNCTION (this);
  m_phy = phy;
  CompleteConfig ();
}

void
BleNetDevice::SetChannel (Ptr<SpectrumChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);
  m_phy->SetChannel (channel);
  channel->AddRx (m_phy);
  CompleteConfig ();
}

Ptr<BleMac>
BleNetDevice::GetMac (void) const
{
  // NS_LOG_FUNCTION (this);
  return m_mac;
}

Ptr<BlePhy>
BleNetDevice::GetPhy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

void
BleNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}

uint32_t
BleNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}

Ptr<Channel>
BleNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy->GetChannel ();
}

void
BleNetDevice::LinkUp (void)
{
  NS_LOG_FUNCTION (this);
  m_linkUp = true;
  m_linkChanges ();
}

void
BleNetDevice::LinkDown (void)
{
  NS_LOG_FUNCTION (this);
  m_linkUp = false;
  m_linkChanges ();
}

Ptr<SpectrumChannel>
BleNetDevice::DoGetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy->GetChannel ();
}

void
BleNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this);
  m_mac->SetShortAddress (Mac16Address::ConvertFrom (address));
}

Address
BleNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac->GetShortAddress ();
}

bool
BleNetDevice::SetMtu (const uint16_t mtu)
{
  NS_ABORT_MSG ("Unsupported");
  return false;
}

uint16_t
BleNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  // Maximum payload size is: max psdu - frame control - seqno - addressing - security - fcs
  //                        = 127      - 2             - 1     - (2+2+2+2)  - 0        - 2
  //                        = 114
  // assuming no security and addressing with only 16 bit addresses without pan id compression.
  return 114;
}

bool
BleNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy != 0 && m_linkUp;
}

void
BleNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  m_linkChanges.ConnectWithoutContext (callback);
}

bool
BleNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
BleNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac16Address ("ff:ff");
}

bool
BleNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
BleNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_ABORT_MSG ("Unsupported");
  return Address ();
}

Address
BleNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this);
  /* Implementation based on RFC 4944 Section 9.
   * An IPv6 packet with a multicast destination address (DST),
   * consisting of the sixteen octets DST[1] through DST[16], is
   * transmitted to the following 802.15.4 16-bit multicast address:
   *           0                   1
   *           0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
   *          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *          |1 0 0|DST[15]* |   DST[16]     |
   *          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * Here, DST[15]* refers to the last 5 bits in octet DST[15], that is,
   * bits 3-7 within DST[15].  The initial 3-bit pattern of "100" follows
   * the 16-bit address format for multicast addresses (Section 12). */

  // \todo re-add this once ble will be able to accept these multicast addresses
  //  uint8_t buf[16];
  //  uint8_t buf2[2];
  //
  //  addr.GetBytes(buf);
  //
  //  buf2[0] = 0x80 | (buf[14] & 0x1F);
  //  buf2[1] = buf[15];
  //
  //  Mac16Address newaddr = Mac16Address();
  //  newaddr.CopyFrom(buf2);
  //  return newaddr;

  return Mac16Address ("ff:ff");
}

bool
BleNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
BleNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
BleNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  // This method basically assumes an 802.3-compliant device, but a raw
  // 802.15.4 device does not have an ethertype, and requires specific
  // McpsDataRequest parameters.
  // For further study:  how to support these methods somehow, such as
  // inventing a fake ethertype and packet tag for McpsDataRequest
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  if (packet->GetSize () > GetMtu ())
    {
      NS_LOG_ERROR ("Fragmentation is needed for this packet, drop the packet ");
      return false;
    }

  McpsDataRequestParams m_mcpsDataRequestParams;
  m_mcpsDataRequestParams.m_dstAddr = Mac16Address::ConvertFrom (dest);
  m_mcpsDataRequestParams.m_dstAddrMode = SHORT_ADDR;
  m_mcpsDataRequestParams.m_dstPanId = m_mac->GetPanId ();
  m_mcpsDataRequestParams.m_srcAddrMode = SHORT_ADDR;
  // Using ACK requests for broadcast destinations is ok here. They are disabled
  // by the MAC.
  if (m_useAcks)
    {
      m_mcpsDataRequestParams.m_txOptions = TX_OPTION_ACK;
    }
  m_mcpsDataRequestParams.m_msduHandle = 0;
  m_mac->McpsDataRequest (m_mcpsDataRequestParams, packet);
  return true;
}

bool
BleNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_ABORT_MSG ("Unsupported");
  // TODO: To support SendFrom, the MACs McpsDataRequest has to use the provided source address, instead of to local one.
  return false;
}

Ptr<Node>
BleNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void
BleNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  m_node = node;
  CompleteConfig ();
}

bool
BleNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
BleNetDevice::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_receiveCallback = cb;
}

void
BleNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  // This method basically assumes an 802.3-compliant device, but a raw
  // 802.15.4 device does not have an ethertype, and requires specific
  // McpsDataIndication parameters.
  // For further study:  how to support these methods somehow, such as
  // inventing a fake ethertype and packet tag for McpsDataRequest
  NS_LOG_WARN ("Unsupported; use Ble MAC APIs instead");
}

void
BleNetDevice::McpsDataIndication (McpsDataIndicationParams params, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this);
  // TODO: Use the PromiscReceiveCallback if the MAC is in promiscuous mode.
  m_receiveCallback (this, pkt, 0, params.m_srcAddr);
}

bool
BleNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

int64_t
BleNetDevice::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (stream);
  int64_t streamIndex = stream;
  streamIndex += m_phy->AssignStreams (stream);
  NS_LOG_DEBUG ("Number of assigned RV streams:  " << (streamIndex - stream));
  return (streamIndex - stream);
}

} // namespace ns3
