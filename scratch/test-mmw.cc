#include <iomanip>
#include <fstream>
#include <string>
#include "ns3/mmwave-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/buildings-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;
using namespace mmwave;

#define SCRIPT_SECTION 1  // Used to make sections collapsible 

//======================================================================
//===> Misc functions and helpers <=====================================

#ifdef SCRIPT_SECTION



static double CurrentTime () { 
  return Simulator::Now ().GetSeconds (); 
}

static void ReportTime () {
  static double limit = 0.1;
  if (CurrentTime () >= limit) {
    limit += 0.1;
    std::cout << std::setprecision (5)
              << "    Time: "  
              << CurrentTime () 
              << "        \n"  
              << std::flush;
  }  
  else {
    std::cout << std::setprecision (5)
             << "    Time: "  
             << CurrentTime () 
             << "        \r"  
             << std::flush;
  }  
    
  Simulator::Schedule (Seconds (0.001), &ReportTime);  
}

void LogHeader (std::string header) {
  std::cout << "\n*** " << header << std::endl;
}

void LogHeader (std::string header, std::string value) {
  std::cout << "\n*** " << header << ": " << value << std::endl;
}

void LogHeader (std::string header, int value) {
  std::cout << "\n*** " << header << ": " << value << std::endl;
}

void LogHeader (std::string header, uint32_t value) {
  std::cout << "\n*** " << header << ": " << value << std::endl;
}

void LogParam (std::string param, std::string value) {
  std::cout << "    " << param << ": " << value << std::endl;
}

void LogParam (std::string param, int value) {
  std::cout << "    " << param << ": " << value << std::endl;
}

void LogParam (std::string param, uint32_t value) {
  std::cout << "    " << param << ": " << value << std::endl;
}

void LogParam (std::string param, double value) {
  std::cout << "    " << param << ": " << value << std::endl;
}

void LogParam (std::string param, bool value) {
  std::cout << "    " << param << ": " << value << std::endl;
}

void LogParam (std::string param, const ns3::Ipv4Address& addr) {
  std::cout << "    " << param << ": " << addr << std::endl;
}

void LogParam (std::string param, ns3::Ipv4Mask& mask) {
  std::cout << "    " << param << ": " << mask << std::endl;
}

void LogParam (std::string param, Box& b) {
  std::cout << "    " << param << ": box coordinates:" << std::endl;
  std::cout << "    X: " << b.xMin << " > " << b.xMax << std::endl;
  std::cout << "    Y: " << b.yMin << " > " << b.yMax << std::endl;
  std::cout << "    Z: " << b.zMin << " > " << b.zMax << std::endl;
}

void LogParam (std::string param, Vector& v) {
  std::cout << "    " << param << ": vector coordinates:" << std::endl;
  std::cout << "    " << v.x << " " << v.y << " " << v.z << std::endl;
}

std::string AddrToStr (Ipv4Address addr) {
  std::ostringstream sstr;
  addr.Print (sstr);
  return sstr.str ();
}

std::string DoubleToStr (double value) {
  std::stringstream sstr;
  sstr << std::fixed << value;
  return sstr.str ();
}

std::string IntToStr (int value) {
  std::stringstream sstr;
  sstr << value;
  return sstr.str ();
}

std::string UintToStr (uint32_t value) {
  std::stringstream sstr;
  sstr << value;
  return sstr.str();
}

#endif //===> End of functions and helpers section <====================

//======================================================================
//===> Config parsing section <=========================================

#ifdef SCRIPT_SECTION

class MyAppTag : public Tag
{
public:
  MyAppTag ()
  {
  }

  MyAppTag (Time sendTs) : m_sendTs (sendTs)
  {
  }

  static TypeId GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MyAppTag")
      .SetParent<Tag> ()
      .AddConstructor<MyAppTag> ();
    return tid;
  }

  virtual TypeId  GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  virtual void  Serialize (TagBuffer i) const
  {
    i.WriteU64 (m_sendTs.GetNanoSeconds ());
  }

  virtual void  Deserialize (TagBuffer i)
  {
    m_sendTs = NanoSeconds (i.ReadU64 ());
  }

  virtual uint32_t  GetSerializedSize () const
  {
    return sizeof (m_sendTs);
  }

  virtual void Print (std::ostream &os) const
  {
    std::cout << m_sendTs;
  }

  Time m_sendTs;
};


class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();
  void ChangeDataRate (DataRate rate);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::ChangeDataRate (DataRate rate)
{
  m_dataRate = rate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  MyAppTag tag (Simulator::Now ());

  m_socket->Send (packet);
  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}



void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

typedef struct ScriptConfig {
  double m_simTime;
  double m_pktInterval;
  double m_minDistance;
  double m_maxDistance;
  double m_frequency;
  std::string m_e2eProt;
  std::string m_simName;
  std::string m_traceDir;
  std::string cc_prot;
  uint16_t m_numEnb;
  uint16_t m_numUe;
  uint32_t m_seed;
  uint32_t m_run;
  uint32_t m_symPerSf;
  uint32_t m_rlcBufSize;
  bool m_rlcAmEnabled;
  bool m_harqEnabled;
  bool m_fixedTti;
  bool m_smallScale;
  double m_sfPeriod;
  double m_speed;
  bool m_isRef;
} ScriptConfig;

typedef struct ScriptHolder {
  NodeContainer m_ueNodes;
  NodeContainer m_enbNodes;
  NodeContainer m_srvNodes;
  NetDeviceContainer m_ueDevs;
  NetDeviceContainer m_enbDevs;
  Ipv4InterfaceContainer m_ueIntfs;
  Ptr<MmWavePointToPointEpcHelper> m_epcHelper;
  Ptr<MmWaveHelper> m_mmwHelper;
  Ptr<Node> m_pgwNode;
} ScriptHolder;

void ParseArgs (ScriptConfig *c, int argc, char *argv[]) {
  c->m_traceDir = "traces/";
  c->m_simName = "test";
  c->m_simTime = 15;
  c->m_pktInterval = 100;           // 500 microseconds
  c->m_frequency = 28e9;
  c->m_rlcBufSize = 7e6;
  c->m_numEnb = 1;
  c->m_numUe = 1;
  c->m_seed = 2;
  c->m_run = 0;
  c->m_symPerSf = 24;
  c->m_harqEnabled = true;
  c->m_rlcAmEnabled = true;
  c->m_fixedTti = false;
  c->m_smallScale = true;
  c->m_sfPeriod = 100.0;
  c->m_speed = 3;
  c->m_isRef = false;
  c->cc_prot = "TcpBbr";
  
  CommandLine cmd;
  cmd.AddValue ("name", "Name used for tracing", c->m_simName);
  cmd.AddValue ("time", "Total duration of the simulation [s])", c->m_simTime);
  cmd.AddValue ("numEnb", "Number of eNBs", c->m_numEnb);
  cmd.AddValue ("numUe", "Number of UEs per eNB", c->m_numUe);
  cmd.AddValue ("e2e", "Protocol used end-to-end", c->m_e2eProt);
  cmd.AddValue ("interPacketInterval", "Inter-packet interval [us])", c->m_pktInterval);
  cmd.AddValue ("harq", "Enable Hybrid ARQ", c->m_harqEnabled);
  cmd.AddValue ("rlcAm", "Enable RLC-AM", c->m_rlcAmEnabled);
  cmd.AddValue ("symPerSf", "OFDM symbols per subframe", c->m_symPerSf);
  cmd.AddValue ("sfPeriod", "Subframe period = 4.16 * symPerSf", c->m_sfPeriod);
  cmd.AddValue ("fixedTti", "Fixed TTI scheduler", c->m_fixedTti);
  cmd.AddValue ("run", "Run for RNG ", c->m_run);
  cmd.AddValue ("isRef", "Reference or modified network stack", c->m_isRef);
  cmd.AddValue ("seed", "The seed that is used in the Simulation", c->m_seed);
  cmd.AddValue ("ccProt" ,"Congestion Control protocol (e.g. TcpSiad, TcpBbr, TcpCubic, etc. used", c->cc_prot);
  cmd.AddValue ("lteBuff", "LTE buffer size", c->m_rlcBufSize);
  cmd.Parse (argc, argv);

  c->m_traceDir += c->m_simName + "/";
  
  LogHeader ("Program arguments parsed");
  LogParam ("Simulation name", c->m_simName);
  LogParam ("Duration", c->m_simTime);
  LogParam ("End-to-end", c->m_e2eProt);
}

void SetDefault (const ScriptConfig &c) {

  //TCP
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName ("ns3::" + c.cc_prot)));
  Config::SetDefault ("ns3::TcpSocketBase::MinRto", TimeValue (MilliSeconds (1000)));
  //Config::SetDefault ("ns3::Ipv4L3Protocol::FragmentExpirationTimeout", TimeValue (Seconds (0.2)));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (10000));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (131072 * 50)); //Default times 50
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (131072 * 50)); //Default times 50

  //Config::SetDefault ("ns3::UdpSocket::RcvBufSize", UintegerValue (131072 * 50)); //Default times 50

  Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue (c.m_rlcAmEnabled));
  Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue (c.m_harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (c.m_harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::CqiTimerThreshold", UintegerValue (1000));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::HarqEnabled", BooleanValue (c.m_harqEnabled));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::FixedTti", BooleanValue (c.m_fixedTti));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::SymPerSlot", UintegerValue (6));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::ResourceBlockNum", UintegerValue (1));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue (72));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::SymbolsPerSubframe", UintegerValue (c.m_symPerSf));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::SubframePeriod", DoubleValue (c.m_sfPeriod));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::TbDecodeLatency", UintegerValue (200.0));
  Config::SetDefault ("ns3::MmWaveBeamforming::LongTermUpdatePeriod", TimeValue (MilliSeconds (100.0)));
  Config::SetDefault ("ns3::LteEnbRrc::SystemInformationPeriodicity", TimeValue (MilliSeconds (5.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MicroSeconds (100.0)));
  Config::SetDefault ("ns3::LteRlcUmLowLat::ReportBufferStatusTimer", TimeValue (MicroSeconds (100.0)));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));
  Config::SetDefault ("ns3::LteEnbRrc::FirstSibTime", UintegerValue (2));
  Config::SetDefault ("ns3::MmWaveBeamforming::SmallScaleFading", BooleanValue (c.m_smallScale));
  Config::SetDefault ("ns3::MmWaveBeamforming::FixSpeed", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveBeamforming::UeSpeed", DoubleValue (c.m_speed));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq",DoubleValue (c.m_frequency));
  //KZS
  Config::SetDefault ("ns3::LteRlcAm::EnableAQM", BooleanValue(false));
  //Config::SetDefault ("ns3::CoDelQueueDisc::Target", StringValue("5ms"));
  Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (c.m_rlcBufSize));
  Config::SetDefault ("ns3::MmWaveHelper::PathlossModel", StringValue ("ns3::BuildingsObstaclePropagationLossModel"));

  /*Config::SetDefault ("ns3::MmWavePhyMacCommon::ResourceBlockNum", UintegerValue (1));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue (72));
  Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::NumHarqProcess", UintegerValue(100));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveBeamforming::LongTermUpdatePeriod", TimeValue (MilliSeconds (100.0)));
  Config::SetDefault ("ns3::LteRlcAm::PollRetransmitTimer", TimeValue (MilliSeconds (4.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReorderingTimer", TimeValue (MilliSeconds (2.0)));
  Config::SetDefault ("ns3::LteRlcAm::StatusProhibitTimer", TimeValue (MilliSeconds (1.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MilliSeconds (4.0)));
  Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (20 * 1024 * 1024));
  Config::SetDefault ("ns3::LteRlcAm::EnableAQM", BooleanValue(true));
  Config::SetDefault ("ns3::CoDelQueueDisc::Target", StringValue("5ms"));
  Config::SetDefault ("ns3::MmWaveHelper::PathlossModel", StringValue ("ns3::MmWavePropagationLossModel"));*/
}

void CreateNodes (const ScriptConfig &c, ScriptHolder *h) {
  h->m_ueNodes.Create (c.m_numUe);
  h->m_enbNodes.Create (c.m_numEnb);
  
  h->m_epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  h->m_mmwHelper = CreateObject<MmWaveHelper> ();
  h->m_mmwHelper->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
  h->m_mmwHelper->SetEpcHelper (h->m_epcHelper);
  h->m_mmwHelper->SetHarqEnabled (c.m_harqEnabled);
  h->m_pgwNode = h->m_epcHelper->GetPgwNode ();

  h->m_srvNodes.Create (c.m_numUe);

  LogHeader ("Creating nodes");
  for (int i = 0; i < c.m_numUe; i++) {
    LogParam ("UE node", h->m_ueNodes.Get (i)->GetId ());
  }
  for (int i = 0; i < c.m_numEnb; i++) {
    LogParam ("ENB node", h->m_enbNodes.Get (i)->GetId ());
  }
  LogParam ("PGW node", h->m_pgwNode->GetId ());
  for (int i = 0; i < c.m_numUe; i++) {
    LogParam ("SRV node", h->m_srvNodes.Get (i)->GetId ());
  }
}

#endif //===> End of config parsing section <===========================

//======================================================================
//===> P2P trace section <==============================================

#ifdef SCRIPT_SECTION

static void T_MAC_QUEUE_PKT (Ptr<OutputStreamWrapper> tracer, uint32_t old, uint32_t size) {
  *tracer->GetStream () << "MQP:" << Simulator::Now ().GetSeconds ()                           
                        << ":"    << size << std::endl;
}

static void T_RX_PHY_DROP (Ptr<OutputStreamWrapper> tracer, Ptr<const Packet> pkt) {
  Ptr<Packet> copy = pkt->Copy (); 
  PppHeader ppp;
  Ipv4Header ip;
  copy->RemoveHeader (ppp);
  copy->RemoveHeader (ip);
  if (ip.GetProtocol () == 6) {
    TcpHeader tcp;
    copy->RemoveHeader (tcp);
    *tracer->GetStream () << "RXPD:" << Simulator::Now ().GetSeconds ()
                          << ":"     << pkt->GetSize ()
                          << ":"     << tcp.GetSequenceNumber () 
                          << ":"     << tcp.GetAckNumber () 
                          << ":"     << tcp.GetSourcePort () 
                          << ":"     << tcp.GetDestinationPort ()                               
                          << ":"     << ip.GetSource ()                               
                          << ":"     << ip.GetDestination () << std::endl;  
  }                                              
  else if (ip.GetProtocol () == 17) {
    UdpHeader udp;
    copy->RemoveHeader (udp);
    *tracer->GetStream () << "RXPD:" << Simulator::Now ().GetSeconds ()
                          << ":"     << pkt->GetSize ()
                          << ":"     << 0
                          << ":"     << 0
                          << ":"     << udp.GetSourcePort () 
                          << ":"     << udp.GetDestinationPort ()                               
                          << ":"     << ip.GetSource ()                               
                          << ":"     << ip.GetDestination () << std::endl;  
  }
  else {
    *tracer->GetStream () << "RXPDU"   
                          << ":" << Simulator::Now ().GetSeconds ()
                          << ":" << pkt->GetSize ()              
                          << ":" << ip.GetSource ()                               
                          << ":" << ip.GetDestination () << std::endl;  
  }
}

static void T_PPP_PKT (Ptr<OutputStreamWrapper> tracer, Ptr<const Packet> pkt, std::string pre) {
  Ptr<Packet> copy = pkt->Copy (); 
  PppHeader ppp;
  Ipv4Header ip;
  copy->RemoveHeader (ppp);
  copy->RemoveHeader (ip);
  if (ip.GetProtocol () == 6) {
    TcpHeader tcp;
    copy->RemoveHeader (tcp);
    *tracer->GetStream () << pre 
                          << ":" << Simulator::Now ().GetSeconds ()
                          << ":" << pkt->GetSize ()
                          << ":" << tcp.GetSequenceNumber () 
                          << ":" << tcp.GetAckNumber () 
                          << ":" << tcp.GetSourcePort () 
                          << ":" << tcp.GetDestinationPort ()                               
                          << ":" << ip.GetSource ()                               
                          << ":" << ip.GetDestination () << std::endl;
  }                                              
  else if (ip.GetProtocol () == 17) {
    UdpHeader udp;
    copy->RemoveHeader (udp);
    *tracer->GetStream () << pre 
                          << ":" << Simulator::Now ().GetSeconds ()
                          << ":" << pkt->GetSize ()
                          << ":" << "x"
                          << ":" << "x"
                          << ":" << udp.GetSourcePort () 
                          << ":" << udp.GetDestinationPort ()                               
                          << ":" << ip.GetSource ()                               
                          << ":" << ip.GetDestination () << std::endl;
  }  
  else {
    *tracer->GetStream () << pre 
                          << ":" << Simulator::Now ().GetSeconds ()
                          << ":" << pkt->GetSize ()                   
                          << ":" << "x"                 
                          << ":" << "x"                 
                          << ":" << "x"                 
                          << ":" << "x"                 
                          << ":" << ip.GetSource ()                               
                          << ":" << ip.GetDestination () << std::endl;  
  }
}

static void T_TX_MAC_DROP (Ptr<OutputStreamWrapper> tracer, Ptr<const Packet> pkt) {
  T_PPP_PKT (tracer, pkt, "TXMD");
}

static void T_TX_PPP (Ptr<OutputStreamWrapper> tracer, Ptr<const Packet> pkt) {
  T_PPP_PKT (tracer, pkt, "TX");
}

static void T_RX_PPP (Ptr<OutputStreamWrapper> tracer, Ptr<const Packet> pkt) {
  T_PPP_PKT (tracer, pkt, "RX");
}

static void T_TX_PPP_TNL (Ptr<OutputStreamWrapper> tracer, Ptr<const Packet> pkt) {
  T_PPP_PKT (tracer, pkt, "TXT");
}

static void T_RX_PPP_TNL (Ptr<OutputStreamWrapper> tracer, Ptr<const Packet> pkt) {
  T_PPP_PKT (tracer, pkt, "RXT");
}

void TraceLink (ScriptConfig& c, Ptr<NetDevice> dev, std::string id) {
  Ptr<PointToPointNetDevice> p2pDev;
  p2pDev = DynamicCast<PointToPointNetDevice>(dev);

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> tracer;
  std::string cmd = "mkdir -p " + c.m_traceDir;
  if (system (cmd.c_str ())) { 
  } 
  tracer = asciiTraceHelper.CreateFileStream (c.m_traceDir + id);

  p2pDev->TraceConnectWithoutContext("MacTx",          MakeBoundCallback (&T_TX_PPP,        tracer));
  p2pDev->TraceConnectWithoutContext("MacRx",          MakeBoundCallback (&T_RX_PPP,        tracer));
  p2pDev->TraceConnectWithoutContext("MacTxTnl",       MakeBoundCallback (&T_TX_PPP_TNL,    tracer));
  p2pDev->TraceConnectWithoutContext("MacRxTnl",       MakeBoundCallback (&T_RX_PPP_TNL,    tracer));
  p2pDev->TraceConnectWithoutContext("MacTxDrop",      MakeBoundCallback (&T_TX_MAC_DROP,   tracer));
  p2pDev->TraceConnectWithoutContext("PhyRxDrop",      MakeBoundCallback (&T_RX_PHY_DROP,   tracer));
}

/*void TraceQueue (ScriptConfig& c, Ptr<NetDevice> dev, std::string id) {
  Ptr<PointToPointNetDevice> p2pDev;
  p2pDev = DynamicCast<PointToPointNetDevice>(dev);
  Ptr<Queue<Packet> > p2pBuf = p2pDev->GetQueue ();

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> tracer;
  std::string cmd = "mkdir -p " + c.m_traceDir;
  if (system (cmd.c_str ())) { 
  } 
  tracer = asciiTraceHelper.CreateFileStream (c.m_traceDir + id);

  p2pBuf->TraceConnectWithoutContext("PacketsInQueue", MakeBoundCallback (&T_MAC_QUEUE_PKT, tracer));
}*/

#endif //===> End of trace section <====================================

//======================================================================
//===> Wireless trace section <=========================================

#ifdef SCRIPT_SECTION

static void T_IP_PKT (Ptr<OutputStreamWrapper> tracer, Ptr<const Packet> pkt, std::string pre) {          
  Ptr<Packet> copy = pkt->Copy (); 
  Ipv4Header ip;
  copy->RemoveHeader (ip);
  if (ip.GetProtocol () == 6) {
    TcpHeader tcp;
    copy->RemoveHeader (tcp);
    *tracer->GetStream () << pre
                          << ":" << Simulator::Now ().GetSeconds ()
                          << ":" << pkt->GetSize ()
                          << ":" << tcp.GetSequenceNumber () 
                          << ":" << tcp.GetAckNumber () 
                          << ":" << tcp.GetSourcePort () 
                          << ":" << tcp.GetDestinationPort ()                               
                          << ":" << ip.GetSource ()                               
                          << ":" << ip.GetDestination () << std::endl;        
  }                                              
  else if (ip.GetProtocol () == 17) {
    UdpHeader udp;
    copy->RemoveHeader (udp);
    *tracer->GetStream () << pre
                          << ":" << Simulator::Now ().GetSeconds ()
                          << ":" << pkt->GetSize ()
                          << ":" << 0
                          << ":" << 0
                          << ":" << udp.GetSourcePort () 
                          << ":" << udp.GetDestinationPort ()                               
                          << ":" << ip.GetSource ()                               
                          << ":" << ip.GetDestination () << std::endl;  
  }
  else {
    *tracer->GetStream () << pre 
                          << ":" << Simulator::Now ().GetSeconds ()
                          << ":" << pkt->GetSize ()                   
                          << ":" << "x"                 
                          << ":" << "x"                 
                          << ":" << "x"                 
                          << ":" << "x"                 
                          << ":" << ip.GetSource ()                               
                          << ":" << ip.GetDestination () << std::endl; 
  }                                          
}

static void T_IP_TX (Ptr<OutputStreamWrapper> tracer, Ptr<const Packet> pkt) {
  T_IP_PKT (tracer, pkt, "TX");
}

static void T_IP_RX (Ptr<OutputStreamWrapper> tracer, Ptr<const Packet> pkt) {
  T_IP_PKT (tracer, pkt, "RX");
}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}


static void
RttChange (Ptr<OutputStreamWrapper> stream, Time oldRtt, Time newRtt)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldRtt.GetSeconds () << "\t" << newRtt.GetSeconds () << std::endl;
}

static void Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << packet->GetSize () << std::endl;
}

void TraceMmwUe (ScriptConfig& c, Ptr<NetDevice> dev, std::string id) {
  Ptr<MmWaveNetDevice> lteDev = DynamicCast<MmWaveNetDevice> (dev);

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> tracer;
  std::string cmd = "mkdir -p " + c.m_traceDir;
  if (system (cmd.c_str ())) { 
  } 
  tracer = asciiTraceHelper.CreateFileStream (c.m_traceDir + id);
  
  lteDev->TraceConnectWithoutContext ("MacTx", MakeBoundCallback (&T_IP_TX, tracer));
  lteDev->TraceConnectWithoutContext ("MacRx", MakeBoundCallback (&T_IP_RX, tracer));
}

void TraceMmwEnb (ScriptConfig& c, Ptr<NetDevice> dev, std::string id) {
  Ptr<MmWaveNetDevice> lteDev = DynamicCast<MmWaveNetDevice> (dev);

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> tracer;
  std::string cmd = "mkdir -p " + c.m_traceDir;
  if (system (cmd.c_str ())) { 
  } 
  tracer = asciiTraceHelper.CreateFileStream (c.m_traceDir + id);
  
  lteDev->TraceConnectWithoutContext ("MacTx", MakeBoundCallback (&T_IP_TX, tracer));
  lteDev->TraceConnectWithoutContext ("MacRx", MakeBoundCallback (&T_IP_RX, tracer));
}

#endif //===> End of trace section <====================================

//======================================================================
//===> Network section <================================================

#ifdef SCRIPT_SECTION

void SetupNs3NetworkStack (NodeContainer n) {
  InternetStackHelper internet;
  internet.Install (n); 
}

void AddNs3Gateway (Ptr<Node> n, Ipv4Address addr, uint32_t via) {
  Ptr<Ipv4StaticRouting> route;
  Ipv4StaticRoutingHelper routing;
  route = routing.GetStaticRouting (n->GetObject<Ipv4> ());
  route->SetDefaultRoute (addr, via);
  
  LogHeader ("Default gateway set");
  LogParam ("At node", n->GetId ());
  LogParam ("Gateway", addr);
  LogParam ("Via device", via);
}

void AddNs3Route (Ptr<Node> n, Ipv4Address addr, Ipv4Mask mask, uint32_t via) {
  Ptr<Ipv4StaticRouting> route;
  Ipv4StaticRoutingHelper routing;
  route = routing.GetStaticRouting (n->GetObject<Ipv4> ());
  route->AddNetworkRouteTo (addr, mask, via);
	
  LogHeader ("Route added");
  LogParam ("At node", n->GetId ());
  LogParam ("To", addr);
  LogParam ("Mask", mask);
  LogParam ("Via device", via);
}

#endif //===> End of network section <==================================

//======================================================================
//===> Link section <===================================================

#ifdef SCRIPT_SECTION

typedef struct LinkConfig {
  std::string m_netAddr;
  std::string m_netMask;
  std::string m_dataRate;
  double m_delay;
  uint32_t m_mtu;
} LinkConfig;

typedef struct LinkHolder {
  Ptr<Node> m_node1;
  Ptr<Node> m_node2;
  Ipv4Address m_addr1;
  Ipv4Address m_addr2;
  NetDeviceContainer m_devs;
  Ipv4InterfaceContainer m_intfs;
} LinkHolder;

void LinkNodes (const LinkConfig &c, LinkHolder* h) {
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (c.m_dataRate)));
  p2p.SetDeviceAttribute ("Mtu", UintegerValue (c.m_mtu));
  p2p.SetChannelAttribute ("Delay", TimeValue (Seconds (c.m_delay)));
  //p2p.SetQueue ("ns3::DropTailQueue", "MaxPackets", UintegerValue(20000));
  p2p.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize ("20000p")));
  h->m_devs = p2p.Install (h->m_node1, h->m_node2);
  //KZS
  Ipv4AddressHelper ipv4;
  ipv4.SetBase (c.m_netAddr.c_str (), c.m_netMask.c_str ());
  h->m_intfs = ipv4.Assign (h->m_devs);
  h->m_addr1 = h->m_intfs.GetAddress (0);
  h->m_addr2 = h->m_intfs.GetAddress (1);
  
  //p2p.EnablePcapAll ("mmw-ns3-test");
  
  LogHeader ("New p2p link created");
  LogParam ("Node 1", h->m_node1->GetId ());
  LogParam ("Node 2", h->m_node2->GetId ());
  LogParam ("Address 1", h->m_addr1);
  LogParam ("Address 2", h->m_addr2);
  LogParam ("Data rate", c.m_dataRate);
  LogParam ("Delay", c.m_delay);
  LogParam ("MTU", c.m_mtu);
}

#endif //===> End of link section <=====================================

//======================================================================
//===> Application section <============================================

#ifdef SCRIPT_SECTION

typedef struct AppConfig {
  bool m_isDownload;
  uint16_t m_serverPort;
  uint16_t m_clientPort;
  uint32_t m_pktInterval;
  double m_startTime;
  double m_stopTime;
  Ipv4Address m_serverAddr;
  Ipv4Address m_clientAddr;
} AppConfig;

typedef struct AppHolder {
  Ptr<Node> m_server;
  Ptr<Node> m_client;
  ApplicationContainer m_serverApps;
  ApplicationContainer m_clientApps;
} AppHolder;

void SetupNs3UdpApp (const AppConfig &c, AppHolder *h) {
  UdpClientHelper srv (c.m_clientAddr, c.m_clientPort);
  srv.SetAttribute ("Interval", TimeValue (MicroSeconds (c.m_pktInterval)));
  srv.SetAttribute ("MaxPackets", UintegerValue (1e6));
  h->m_serverApps.Add (srv.Install (h->m_server));
  h->m_serverApps.Start (Seconds (0.01));
  
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), c.m_clientPort));
  h->m_clientApps.Add (packetSinkHelper.Install (h->m_client));
  h->m_clientApps.Start (Seconds (0.01));
  
  LogHeader ("Constant bitrate UDP application created");
  LogParam ("Server node", h->m_server->GetId ());
  LogParam ("Client node", h->m_client->GetId ());
  LogParam ("Client address", c.m_clientAddr);
  LogParam ("Client port", c.m_clientPort);
}

void SetupNs3TcpApp (const AppConfig &c, AppHolder *h, const ScriptConfig &sc, std::string id) {
  //Address sinkAddress (InetSocketAddress (ueIpIface.GetAddress (0), sinkPort));
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (h->m_server, TcpSocketFactory::GetTypeId ());

  //Timed send app
  Ptr<MyApp> source = CreateObject<MyApp> ();
  source->Setup (ns3TcpSocket, InetSocketAddress(c.m_clientAddr, c.m_clientPort), 1400, 5000000, DataRate ("2500Mb/s"));

  h->m_server->AddApplication (source);
  h->m_serverApps.Add (source);
  h->m_serverApps.Start (Seconds (0.02));

  PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), c.m_clientPort));
  ApplicationContainer sinks = sink.Install (h->m_client);
  h->m_clientApps.Add (sinks);
  h->m_clientApps.Start (Seconds (0.01));

  AsciiTraceHelper asciiTraceHelper;
  std::string cmd = "mkdir -p " + sc.m_traceDir;
  if (system (cmd.c_str ())) {
  }
  Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream (sc.m_traceDir + "mmWave-tcp-window" + id + ".txt");
  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream1));

  Ptr<OutputStreamWrapper> stream4 = asciiTraceHelper.CreateFileStream (sc.m_traceDir + "mmWave-tcp-rtt" + id + ".txt");
  ns3TcpSocket->TraceConnectWithoutContext ("RTT", MakeBoundCallback (&RttChange, stream4));

  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream (sc.m_traceDir + "mmWave-tcp-data" + id + ".txt");
  sinks.Get (0)->TraceConnectWithoutContext ("Rx",MakeBoundCallback (&Rx, stream2));

  LogHeader ("Bulk TCP application created");
  LogParam ("Server node", h->m_server->GetId ());
  LogParam ("Client node", h->m_client->GetId ());
  LogParam ("Client address", c.m_clientAddr);
  LogParam ("Client port", c.m_clientPort);
}

void FlipTxDirection (AppConfig *c, AppHolder *h) {
  Ptr<Node> newServer = h->m_client;
  h->m_client = h->m_server;
  h->m_server = newServer;
  
  u_int16_t newServerPort = c->m_clientPort;
  c->m_clientPort = c->m_serverPort;
  c->m_serverPort = newServerPort;
  
  Ipv4Address newServerAddr = c->m_clientAddr;
  c->m_clientAddr = c->m_serverAddr;
  c->m_serverAddr = newServerAddr;
} 

#endif //===> End of application section <==============================

//======================================================================
//===> Mobility section <===============================================

#ifdef SCRIPT_SECTION

void AddObstacles (const ScriptConfig& c) {
  Box box1 (40, 45, -4, 4, 0, 30);
  Ptr<Building> building1 = Create<Building> ();
  building1->SetBoundaries (box1);
  building1->SetNFloors (4);
  building1->SetNRoomsX (4);
  building1->SetNRoomsY (4);
  building1->SetExtWallsType (Building::ConcreteWithoutWindows);
  
  LogHeader ("Obstacles created");
  LogParam ("Obstacle 1", box1);
}

void SetupEnbMobility (const ScriptConfig& c, Ptr<Node> enb) {
  Vector vector = Vector (0, 0, 30);
  Ptr<ListPositionAllocator> pos;
  pos = CreateObject<ListPositionAllocator> ();
  pos->Add (vector);
  
  MobilityHelper mob;
  mob.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mob.SetPositionAllocator (pos);
  mob.Install (enb);
  BuildingsHelper::Install (enb);
  
  LogHeader ("Location given to ENB");
  LogParam ("Node", enb->GetId ());
  LogParam ("Coordinates", vector);
}

void SetupUeMobility (const ScriptConfig& c, Ptr<Node> ue) {
  MobilityHelper mob;
  mob.SetMobilityModel ("ns3::WaypointMobilityModel");
  mob.Install (ue); 
  Ptr<WaypointMobilityModel> waypointMob;
  waypointMob = ue->GetObject<WaypointMobilityModel> ();
  
  std::string stopTime = DoubleToStr(c.m_simTime) + "s";
  Vector point1 = Vector (50, 15, 1.5);
  Vector point2 = Vector (50, -15, 1.5);
  waypointMob->AddWaypoint (Waypoint (Time ("0s"), point1)); 
  waypointMob->AddWaypoint (Waypoint (Time (stopTime), point2)); 
  BuildingsHelper::Install (ue);

  LogHeader ("Waypoint-mobility given to UE");
  LogParam ("Node", ue->GetId ());
  LogParam ("Point 1", point1);
  LogParam ("Point 2", point2);
  LogParam ("Stop time", stopTime);
}

#endif //===> End of mobility section <=================================

//======================================================================
//===> Script main <====================================================

#ifdef SCRIPT_SECTION

int main (int argc, char *argv[]) {
  ScriptConfig c;
  ScriptHolder h;
  ParseArgs (&c, argc, argv);
  SetDefault (c);
  CreateNodes (c, &h);
  RngSeedManager::SetSeed (c.m_seed);
  RngSeedManager::SetRun (c.m_run);
  
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  
  //KZS
  AddObstacles (c);
  for (uint32_t i = 0; i < h.m_enbNodes.GetN (); i++) {
    SetupEnbMobility (c, h.m_enbNodes.Get (i));
  }
  for (uint32_t i = 0; i < h.m_ueNodes.GetN (); i++) {
    SetupUeMobility (c, h.m_ueNodes.Get (i));
  }
  BuildingsHelper::MakeMobilityModelConsistent ();
  
  h.m_enbDevs = h.m_mmwHelper->InstallEnbDevice (h.m_enbNodes);
  h.m_ueDevs = h.m_mmwHelper->InstallUeDevice (h.m_ueNodes);

  for (uint32_t i = 0; i < h.m_ueDevs.GetN (); i++) {
    //TraceMmwUe (c, h.m_ueDevs.Get (i), "00-at-ue-" + IntToStr (i));
  }
  for (uint32_t i = 0; i < h.m_enbDevs.GetN (); i++) {
    //TraceMmwEnb (c, h.m_enbDevs.Get (i), "01-at-enb-" + IntToStr (i));
  }

  SetupNs3NetworkStack (h.m_srvNodes);
  SetupNs3NetworkStack (h.m_ueNodes);

  h.m_ueIntfs = h.m_epcHelper->AssignUeIpv4Address (h.m_ueDevs);
  h.m_mmwHelper->AttachToClosestEnb (h.m_ueDevs, h.m_enbDevs);
  h.m_mmwHelper->EnableTraces ();

  for (uint32_t i = 0; i < h.m_ueNodes.GetN (); i++) {
    Ptr<Node> ue = h.m_ueNodes.Get (i);
    Ptr<Node> srv = h.m_srvNodes.Get (i);
    
    LinkConfig linkConfig;
    linkConfig.m_netAddr = "2.0.0.0";
    linkConfig.m_netMask = "255.255.0.0";
    linkConfig.m_dataRate = "100Gb/s";
    linkConfig.m_delay = 0.0125;
    linkConfig.m_mtu = 1500;
    LinkHolder linkHolder;
    linkHolder.m_node1 = h.m_pgwNode;
    linkHolder.m_node2 = srv;
    LinkNodes (linkConfig, &linkHolder);
    //TraceLink (c, linkHolder.m_devs.Get(0), "03-at-pgw-" + IntToStr (i));
    //TraceLink (c, linkHolder.m_devs.Get(1), "04-at-srv-" + IntToStr (i));
   
    AppConfig appConfig;
    appConfig.m_startTime = 0.0;
    appConfig.m_stopTime = c.m_simTime;
    appConfig.m_serverPort = 2200;
    appConfig.m_clientPort = 2201;
    appConfig.m_pktInterval = c.m_pktInterval;
    appConfig.m_serverAddr = linkHolder.m_addr2;
    appConfig.m_clientAddr = h.m_ueIntfs.GetAddress (i);
    AppHolder appHolder;
    appHolder.m_server = srv;
    appHolder.m_client = ue;
    //FlipTxDirection (&appConfig, &appHolder);

    AddNs3Route (srv, Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
    AddNs3Gateway (ue, h.m_epcHelper->GetUeDefaultGatewayAddress (), 1);
    SetupNs3TcpApp (appConfig, &appHolder, c, IntToStr (i));

  }

  Simulator::Schedule (Seconds (0.01), &ReportTime);
  Simulator::Stop (Seconds (c.m_simTime+0.1));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

#endif //===> End of main section <=====================================


//======================================================================
//===> Script end <=====================================================
