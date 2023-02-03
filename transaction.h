
#ifndef TRANSACTION_H
#define TRANSACTION_H

namespace ns3 {
class Transaction
    {
        public:

            Transaction(int rsuNodeId, int transId, double timeStamp, double payment, int winnerId);
            Transaction();
            virtual ~Transaction(void);

            int GetRsuNodeId(void) const;
            void SetRsuNodeId(int m_rsuNodeId);

            int GetTransId(void) const;
            void SetTransId(int m_transId);

            int GetTransSizeByte(void) const;
            void SetTransSizeByte(int m_transSizeByte);

            double GetTransTimeStamp(void) const;
            void SetTransTimeStamp(double m_timeStamp);

            double GetPayment(void) const;
            void SetPayment(double m_payment);

            int GetWinnerId(void) const;
            void SetWinnerId(int m_winnerId);

            Transaction& operator = (const Transaction &tranSource);     //Assignment Constructor

            friend bool operator == (const Transaction &tran1, const Transaction &tran2);
            
        
        protected:
            int m_rsuNodeId;
            int m_transId;
            int m_transSizeByte;
            double m_timeStamp;
            double m_payment; 
            int m_winnerId;

    };
}

    #endif /* TRANSACTION_H */