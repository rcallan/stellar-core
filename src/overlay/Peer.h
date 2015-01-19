#ifndef __PEER__
#define __PEER__

// Copyright 2014 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "util/asio.h"
#include "xdrpp/message.h"
#include "generated/StellarXDR.h"
#include "generated/FBAXDR.h"

namespace stellar
{
typedef std::shared_ptr<FBAQuorumSet> FBAQuorumSetPtr;

class Application;
class LoopbackPeer;

class PeerRecord
{
public:
    int mPeerID;
    std::string mIP;
    int mPort;
    int mNumFailures;
    PeerRecord(int id,const std::string& ip,int port,int fails) : mPeerID(id), mIP(ip), mPort(port), mNumFailures(fails){ }
};

/*
 * Another peer out there that we are connected to
 */
class Peer : public std::enable_shared_from_this<Peer>
{
    static bool ipFromStr(std::string ipStr, xdr::opaque_array<4U>& ret);

  public:
    typedef std::shared_ptr<Peer> pointer;

    enum PeerState
    {
        CONNECTING,
        CONNECTED,
        GOT_HELLO,
        CLOSING
    };

    enum PeerRole
    {
        INITIATOR,
        ACCEPTOR
    };

  protected:
    Application& mApp;

    PeerRole mRole;
    PeerState mState;

    std::string mRemoteVersion;
    int mRemoteProtocolVersion;
    int mRemoteListeningPort;
    void recvMessage(StellarMessage const& msg);
    void recvMessage(xdr::msg_ptr const& xdrBytes);

    virtual void recvError(StellarMessage const& msg);
    virtual void recvHello(StellarMessage const& msg);
    void recvDontHave(StellarMessage const& msg);
    void recvGetPeers(StellarMessage const& msg);
    void recvPeers(StellarMessage const& msg);
    void recvGetHistory(StellarMessage const& msg);
    void recvHistory(StellarMessage const& msg);
    void recvGetDelta(StellarMessage const& msg);
    void recvDelta(StellarMessage const& msg);
    void recvGetTxSet(StellarMessage const& msg);
    void recvTxSet(StellarMessage const& msg);
    void recvGetValidations(StellarMessage const& msg);
    void recvValidations(StellarMessage const& msg);
    void recvTransaction(StellarMessage const& msg);
    void recvGetFBAQuorumSet(StellarMessage const& msg);
    void recvFBAQuorumSet(StellarMessage const& msg);
    void recvFBAMessage(StellarMessage const& msg);

    void sendHello();
    void sendFBAQuorumSet(FBAQuorumSetPtr qSet);
    void sendDontHave(MessageType type,
                      uint256 const& itemID);
    void sendPeers();

    // NB: This is a move-argument because the write-buffer has to travel
    // with the write-request through the async IO system, and we might have
    // several queued at once. We have carefully arranged this to not copy
    // data more than the once necessary into this buffer, but it can't be
    // put in a reused/non-owned buffer without having to buffer/queue
    // messages somewhere else. The async write request will point _into_
    // this owned buffer. This is really the best we can do.
    virtual void sendMessage(xdr::msg_ptr&& xdrBytes) = 0;
    virtual void connected() {}
  public:
    Peer(Application& app, PeerRole role);
    
    Application&
    getApp()
    {
        return mApp;
    }

    void sendGetTxSet(uint256 const& setID);
    void sendGetQuorumSet(uint256 const& setID);

    void sendMessage(StellarMessage const& msg);

    PeerRole
    getRole() const
    {
        return mRole;
    }


    
    PeerState
    getState() const
    {
        return mState;
    }

    std::string const&
    getRemoreVersion() const
    {
        return mRemoteVersion;
    }
    
    int
    getRemoteProtocolVersion() const
    {
        return mRemoteProtocolVersion;
    }
    
    int
    getRemoteListeningPort()
    {
        return mRemoteListeningPort;
    }

    // These exist mostly to be overridden in TCPPeer and callable via
    // shared_ptr<Peer> as a captured shared_from_this().
    virtual void connectHandler(const asio::error_code& ec);
    
    virtual void
    writeHandler(const asio::error_code& error, std::size_t bytes_transferred)
    {
    }
    
    virtual void
    readHeaderHandler(const asio::error_code& error,
                      std::size_t bytes_transferred)
    {
    }

    virtual void
    readBodyHandler(const asio::error_code& error,
                    std::size_t bytes_transferred)
    {
    }

    virtual void drop() = 0;
    virtual std::string getIP() = 0;
    virtual ~Peer()
    {
    }

    friend class LoopbackPeer;
};
}

#endif
