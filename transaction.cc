#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/log.h"
#include "transaction.h"

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
}