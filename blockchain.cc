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

    Transaction::Transaction(int rsuNodeId, int transId, double timeStamp, double payment, int winnerId)
    {
        m_rsuNodeId = rsuNodeId;
        m_transId = transId;
        m_transSizeByte = 100;
        m_timeStamp = timeStamp;
        m_payment = payment;
        m_winnerId = winnerId;
        m_validatation = false;
    }
    
    Transaction::Transaction()
    {
        Transaction(0, 0, 0, 0, 0);
    }
    
    Transaction::~Transaction()
    {
    }

    int
    Transaction::GetRsuNodeId(void) const
    {
        return m_rsuNodeId;
    }

    void
    Transaction::SetRsuNodeId(int nodeId)
    {
        m_rsuNodeId = nodeId;
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

    double
    Transaction::GetPayment(void) const
    {
        return m_payment;
    }

    void
    Transaction::SetPayment(double payment)
    {
        m_payment = payment;
    }

    int
    Transaction::GetWinnerId(void) const
    {
        return m_winnerId;
    }

    void
    Transaction::SetWinnerId(int winnerId)
    {
        m_winnerId = winnerId;
    }

    void
    Transaction::SetValidation()
    {
        m_validatation = true;
    }

    Transaction&
    Transaction::operator= (const Transaction &tranSource)
    {
        m_rsuNodeId = tranSource.m_rsuNodeId;
        m_transId = tranSource.m_transId;
        m_transSizeByte = tranSource.m_transSizeByte;
        m_timeStamp = tranSource.m_timeStamp;
        m_payment = tranSource.m_payment;
        m_winnerId = tranSource.m_winnerId;

        return *this;
    }

    bool operator== (const Transaction &tran1, const Transaction &tran2)
    {
        if(tran1.GetRsuNodeId() == tran2.GetRsuNodeId() && tran1.GetTransId() == tran2.GetTransId())
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
            if(tran.GetRsuNodeId()==nodeId && tran.GetTransId() == transId)
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
                if(tran.GetRsuNodeId() == nodeId && tran.GetTransId() == tranId)
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
                    << tran.GetRsuNodeId() << " transId : " << tran.GetTransId() << "\n";
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


    /*
     *
     * Class Blockchain Function
     * 
     */


    Blockchain::Blockchain(void)
    {
        m_totalBlocks = 0;
        Block genesisBlock(0,0,0,0,0,0,0, Ipv4Address("0.0.0.0"));
        AddBlock(genesisBlock);
    }

    Blockchain::~Blockchain(void)
    {
    }

    int
    Blockchain::GetTotalBlocks(void) const
    {
        return m_totalBlocks;
    }

    int
    Blockchain::GetBlockchainHeight(void) const
    {
        return GetCurrentTopBlock()->GetBlockHeight();
    }

    bool
    Blockchain::HasBlock(const Block &newBlock) const
    {
        if(newBlock.GetBlockHeight() > GetCurrentTopBlock()->GetBlockHeight())
        {
            // We didn't receive the new block which have a new block height.
            return false;
        }
        else
        {
            for(auto const &block: m_blocks[newBlock.GetBlockHeight()])
            {
                if(block == newBlock)
                {
                    return true;
                }
            }
        }

        return false;
    }

    bool
    Blockchain::HasBlock(int height, int minerId) const
    {
        if(height > GetCurrentTopBlock()->GetBlockHeight())
        {
            // We didn't receive the new block which have a new block height.
            return false;
        }
        else
        {
            for(auto const &block: m_blocks[height])
            {
                if(block.GetBlockHeight() == height && block.GetMinerId() == minerId)
                {
                    return true;
                }
            }
        }
        return false;
    }

    Block
    Blockchain::ReturnBlock(int height, int minerId)
    {
        std::vector<Block>::iterator block_it;

        if(height <= GetBlockchainHeight() && height >= 0)
        {
            for(block_it = m_blocks[height].begin(); block_it < m_blocks[height].end(); block_it++)
            {
                if(block_it->GetBlockHeight() == height && block_it->GetMinerId() == minerId)
                {
                    return *block_it;
                }
            }
        }

        for(block_it = m_orphans.begin(); block_it < m_orphans.end(); block_it++)
        {
            if(block_it->GetBlockHeight()==height && block_it->GetMinerId()==minerId)
            {
                return *block_it;
            }
        }

        return Block();
    }

    bool
    Blockchain::IsOrphan (const Block &newBlock) const
    {
        for(auto const &block: m_orphans)
        {
            if(block == newBlock)
            {
                return true;
            }
        }
        return false;
    }

    bool
    Blockchain::IsOrphan(int height, int minerId) const
    {
        for(auto const &block: m_orphans)
        {
            if(block.GetBlockHeight()==height && block.GetMinerId()==minerId)
            {
                return true;
            }
        }
        return false;
    }

    const Block*
    Blockchain::GetBlockPointer(const Block &newBlock) const
    {
        for(auto const &block : m_blocks[newBlock.GetBlockHeight()])
        {
            if(block == newBlock)
            {
                return &block;
            }
        }
        
        return nullptr;
    }

    const std::vector<const Block *>
    Blockchain::GetChildrenPointers (const Block &block)
    {
        std::vector<const Block *> children;
        std::vector<Block>::iterator block_it;
        int childrenHeight = block.GetBlockHeight() + 1;

        if(childrenHeight > GetBlockchainHeight())
        {
            return children;
        }

        for(block_it = m_blocks[childrenHeight].begin(); block_it < m_blocks[childrenHeight].end(); block_it++)
        {
            if(block.IsParent(*block_it))
            {
                children.push_back(&(*block_it));
            }
        }

        return children;
    }

    const std::vector<const Block *>
    Blockchain::GetOrpharnChildrenPointer (const Block &block)
    {
        std::vector<const Block *> children;
        std::vector<Block>::iterator block_it;

        for(block_it = m_orphans.begin(); block_it < m_orphans.end(); block_it++)
        {
            if(block.IsParent(*block_it))
            {
                children.push_back(&(*block_it));
            }
        }
        return children;
    }

    const Block*
    Blockchain::GetParent(const Block &block)
    {
        std::vector<Block>::iterator block_it;
        int parentHeight = block.GetBlockHeight() - 1;

        if(parentHeight > GetBlockchainHeight() || parentHeight < 0)
            return nullptr;
        
        for(block_it = m_blocks[parentHeight].begin(); block_it < m_blocks[parentHeight].end(); block_it++)
        {
            if(block.IsChild(*block_it))
            {
                return &(*block_it);
            }
        }
        return nullptr;
    }

    const Block*
    Blockchain::GetCurrentTopBlock(void) const
    {
        return &m_blocks[m_blocks.size()-1][0];
    }

    void
    Blockchain::AddBlock(const Block& newBlock)
    {

        if(m_blocks.size() == 0)
        {
           std::vector<Block> newHeight(1, newBlock);
           m_blocks.push_back(newHeight);
        }
        else if(newBlock.GetBlockHeight() > GetCurrentTopBlock()->GetBlockHeight())
        {
           int dummyRows = newBlock.GetBlockHeight() - GetCurrentTopBlock()->GetBlockHeight()-1;

           for(int i = 0 ; i < dummyRows; i++)
           {
               std::vector<Block> newHeight;
               m_blocks.push_back(newHeight);
           }

           std::vector<Block> newHeight(1, newBlock);
           m_blocks.push_back(newHeight);
        }
        else
        {
            m_blocks[newBlock.GetBlockHeight()].push_back(newBlock);
        }
        m_totalBlocks++;
    }

    void
    Blockchain::AddOrphan(const Block& newBlock)
    {
        m_orphans.push_back(newBlock);
    }

    void
    Blockchain::RemoveOrphan(const Block& newBlock)
    {
        std::vector<Block>::iterator block_it;

        for(block_it = m_orphans.begin(); block_it < m_orphans.end(); block_it++)
        {
            if(newBlock==*block_it)
            {
                break;
            }
        }
        
        if(block_it == m_orphans.end())
        {
            return;
        }
        else
        {
            m_orphans.erase(block_it);
        }
    }



    const char* getMessageName(enum Messages m)
    {
        switch(m)
        {
            case REQUEST_TRANS: return "REQUEST_TRANS";
            case RESPONSE_TRANS: return "RESPONSE_TRANS";
            case REQUEST_BLOCK: return "REQUEST_BLOCK";

        }

        return 0;
    }

    const char* getMinerType(enum MinerType m)
    {
        switch(m)
        {
            case NORMAL_MINER: return "ETHEREUM";
            case HYPERLEDGER_MINER: return "HYPERLEDGER";
        }

        return 0;
    }

    const char* getProtocolType(enum ProtocolType m)
    {
         switch(m)
         {
             case STANDARD_PROTOCOL: return "STANDARD_PROTOCOL";
             case SENDHEADERS: return "SENDHEADERS";
         }

         return 0;
    }

    const char* getCommitterType(enum CommitterType m)
    {
        switch(m)
        {
            case COMMITTER: return "COMMITTER";
            case ENDORSER: return "ENDORSER";
            case CLIENT: return "CLIENT";
        }
        return 0;
    }

    const char* getCryptocurrency(enum Cryptocurrency m)
    {
        switch(m)
        {
            case ETHEREUM: return "ETHEREUM";
            case HYPERLEDGER: return "HYPERLEDGER";
        }
        
        return 0;
    }

    const char* getBlockchainRegion(enum BlockchainRegion m)
    {
        switch(m)
        {
            case NORTH_AMERICA: return "NORTH_AMERICA";
            case EUROPE: return "EUROPE";
            case SOUTH_AMERICA: return "SOUTH_AMERICA";
            case KOREA: return "KOREA";
            case JAPAN: return "JAPAN";
            case AUSTRALIA: return "AUSTRALIA";
            case OTHER: return "OTHER";
        }

        return 0;
    }

    enum BlockchainRegion getBlockchainEnum(uint32_t n)
    {
        switch(n)
        {
            case 0: return NORTH_AMERICA;
            case 1: return EUROPE;
            case 2: return SOUTH_AMERICA;
            case 3: return KOREA;
            case 4: return JAPAN;
            case 5: return AUSTRALIA;
            case 6: return OTHER;
        }
        return OTHER;
    }

}