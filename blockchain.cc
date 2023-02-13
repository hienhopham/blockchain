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



}