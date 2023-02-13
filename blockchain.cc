#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/log.h"
#include "blockchain.h"

namespace ns3{

    /*
     *
     * Class Transaction Function
     * 
     */

    Transaction::Transaction(int nodeId, int transId, double timeStamp)
    {
        m_nodeId = nodeId;
        m_transId = transId;
        m_transSizeByte = 100;
        m_timeStamp = timeStamp;
        m_validatation = false;
        m_execution = 0;
    }
    
    Transaction::Transaction()
    {
        Transaction(0, 0, 0);
    }
    
    Transaction::~Transaction()
    {
    }

    int
    Transaction::GetTransNodeId(void) const
    {
        return m_nodeId;
    }

    void
    Transaction::SetTransNodeId(int nodeId)
    {
        m_nodeId = nodeId;
    }

    int
    Transaction::GetTransId(void) const
    {
        return m_transId;
    }

    void
    Transaction::SetTransId(int transId)
    {
        m_transId = transId;
    }

    int
    Transaction::GetTransSizeByte(void) const
    {
        return m_transSizeByte;
    }

    void
    Transaction::SetTransSizeByte(int transSizeByte)
    {
        m_transSizeByte = transSizeByte;
    }

    double
    Transaction::GetTransTimeStamp(void) const
    {
        return m_timeStamp;
    }

    void
    Transaction::SetTransTimeStamp(double timeStamp)
    {
        m_timeStamp = timeStamp;
    }

    bool
    Transaction::IsValidated(void) const
    {
        return m_validatation;
    }

    void
    Transaction::SetValidation()
    {
        m_validatation = true;
    }

    int
    Transaction::GetExecution(void) const
    {
        return m_execution;
    }

    void
    Transaction::SetExecution(int endoerserId)
    {
        m_execution = endoerserId;
    }


    Transaction&
    Transaction::operator= (const Transaction &tranSource)
    {
        m_nodeId = tranSource.m_nodeId;
        m_transId = tranSource.m_transId;
        m_transSizeByte = tranSource.m_transSizeByte;
        m_timeStamp = tranSource.m_timeStamp;
        m_validatation = tranSource.m_validatation;
        m_validatation = tranSource.m_execution;

        return *this;
    }

    bool operator== (const Transaction &tran1, const Transaction &tran2)
    {
        if(tran1.GetTransNodeId() == tran2.GetTransNodeId() && tran1.GetTransId() == tran2.GetTransId())
            return true;
        else
            return false;
    }

    /*
     *
     * Class Block Function
     * 
     */

    Block::Block(int blockHeight, int minerId, int nonce, int parentBlockMinerId, int blockSizeBytes,
        double timeStamp, double timeReceived, Ipv4Address receivedFromIpv4)
    {
        m_blockHeight = blockHeight;
        m_minerId = minerId;
        m_nonce = nonce;
        m_parentBlockMinerId = parentBlockMinerId;
        m_blockSizeBytes = blockSizeBytes;
        m_timeStamp = timeStamp;
        m_timeReceived = timeReceived;
        m_receivedFromIpv4 = receivedFromIpv4;
        m_totalTransactions = 0;

    }

    Block::Block()
    {
        Block(0,0,0,0,0,0,0, Ipv4Address("0.0.0.0"));
    }

    Block::Block(const Block &blockSource)
    {
        m_blockHeight = blockSource.m_blockHeight;
        m_minerId = blockSource.m_minerId;
        m_nonce = blockSource.m_nonce;
        m_parentBlockMinerId = blockSource.m_parentBlockMinerId;
        m_blockSizeBytes = blockSource.m_blockSizeBytes;
        m_timeStamp = blockSource.m_timeStamp;
        m_timeReceived = blockSource.m_timeReceived;
        m_receivedFromIpv4 = blockSource.m_receivedFromIpv4;
        m_transactions = blockSource.m_transactions;
        m_totalTransactions = 0;
        
    }

    Block::~Block(void)
    {
    }

    int
    Block::GetBlockHeight(void) const
    {
        return m_blockHeight;
    }

    void
    Block::SetBlockHeight(int blockHeight)
    {
        m_blockHeight = blockHeight;
    }

    int
    Block::GetNonce(void) const
    {
        return m_nonce;
    }

    void
    Block::SetNonce(int nonce)
    {
        m_nonce = nonce;
    }

    int
    Block::GetMinerId(void) const
    {
        return m_minerId;
    }

    void
    Block::SetMinerId(int minerId)
    {
        m_minerId = minerId;
    }

    int
    Block::GetParentBlockMinerId(void) const
    {
        return m_parentBlockMinerId;
    }

    void
    Block::SetParentBlockMinerId(int parentBlockMinerId)
    {
        m_parentBlockMinerId = parentBlockMinerId;
    }

    int
    Block::GetBlockSizeBytes(void) const
    {
        return m_blockSizeBytes;
    }

    void
    Block::SetBlockSizeBytes(int blockSizeBytes)
    {
        m_blockSizeBytes = blockSizeBytes;
    }

    double
    Block::GetTimeStamp(void) const
    {
        return m_timeStamp;
    }

    void
    Block::SetTimeStamp(double timeStamp)
    {
        m_timeStamp = timeStamp;
    }

    double
    Block::GetTimeReceived(void) const
    {
        return m_timeReceived;
    }

    Ipv4Address
    Block::GetReceivedFromIpv4(void) const
    {
        return m_receivedFromIpv4;
    }

    void
    Block::SetReceivedFromIpv4(Ipv4Address receivedFromIpv4)
    {
        m_receivedFromIpv4 = receivedFromIpv4;
    }

    std::vector<Transaction>
    Block::GetTransactions(void) const
    {
        return m_transactions;
    }

    void
    Block::SetTransactions(const std::vector<Transaction> &transactions)
    {
        m_transactions = transactions;
    }

    bool
    Block::IsParent(const Block &block) const
    {
        if(GetBlockHeight()==block.GetBlockHeight() - 1 && GetMinerId() == block.GetParentBlockMinerId())
            return true;
        else
            return false;
    }

    bool
    Block::IsChild(const Block &block) const
    {
        if(GetBlockHeight()==block.GetBlockHeight()+1 && GetParentBlockMinerId()==block.GetMinerId())
            return true;
        else
            return false;
    }

    int
    Block::GetTotalTransaction(void) const
    {
        return m_totalTransactions;
    }

    Transaction
    Block::ReturnTransaction(int nodeId, int transId)
    {
        for(auto const &tran: m_transactions)
        {
            if(tran.GetTransNodeId()==nodeId && tran.GetTransId() == transId)
            {
                return tran;
            }
        }
        
        return Transaction();
    }

    bool
    Block::HasTransaction(Transaction &newTran) const
    {
        for(auto const &tran: m_transactions)
        {
                if(tran == newTran)
                {
                    return true;
                }
        }

        return false;
    }

    bool
    Block::HasTransaction(int nodeId, int tranId) const
    {
        for(auto const &tran: m_transactions)
        {
                if(tran.GetTransNodeId() == nodeId && tran.GetTransId() == tranId)
                {
                    return true;
                }
        }
        return false;
    }

    void
    Block::AddTransaction(const Transaction& newTrans)
    {
        m_transactions.push_back(newTrans);
        m_totalTransactions++;
    
    }

    void
    Block::PrintAllTransaction(void)
    {
        if(m_transactions.size() != 0)
        {
            for(auto const &tran: m_transactions)
            {
                std::cout<<"[Blockheight: " <<m_blockHeight << "] Transaction nodeId: " 
                    << tran.GetTransNodeId() << " transId : " << tran.GetTransId() << "\n";
            }
        }
        else
        {
            std::cout<<"[Blockheight: " <<m_blockHeight << "]  do not have transactions\n";
        }

    }

    Block&
    Block::operator= (const Block &blockSource)
    {
        m_blockHeight = blockSource.m_blockHeight;
        m_minerId = blockSource.m_minerId;
        m_nonce = blockSource.m_nonce;
        m_parentBlockMinerId = blockSource.m_parentBlockMinerId;
        m_blockSizeBytes = blockSource.m_blockSizeBytes;
        m_timeStamp = blockSource.m_timeStamp;
        m_timeReceived = blockSource.m_timeReceived;
        m_receivedFromIpv4 = blockSource.m_receivedFromIpv4;

        return *this;
    }

    bool operator== (const Block &block1, const Block &block2)
    {
        if(block1.GetBlockHeight() == block2.GetBlockHeight() && block1.GetMinerId() == block2.GetMinerId())
            return true;
        else
            return false;
    }


}