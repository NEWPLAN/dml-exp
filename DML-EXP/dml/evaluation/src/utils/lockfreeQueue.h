template <class QElmType>
struct qnode
{
    struct qnode *next;
    QElmType data;
};
template <class QElmType>
class LockfreeQueue
{
public:
    LockfreeQueue() { init(); }
    ~LockfreeQueue() { destroy(); }

    bool init()
    {
        m_front = m_rear = new qnode<QElmType>;
        if (!m_front)
            return false;
        m_front->next = 0;
        return true;
    }
    void destroy()
    {
        while (m_front)
        {
            m_rear = m_front->next;
            delete m_front;
            m_front = m_rear;
        }
    }
    bool push(QElmType e)
    {
        struct qnode<QElmType> *p = new qnode<QElmType>;
        if (!p)
            return false;
        p->next = 0;
        m_rear->next = p;
        m_rear->data = e;
        m_rear = p;
        return true;
    }
    bool pop(QElmType *e)
    {
        if (m_front == m_rear)
            return false;

        struct qnode<QElmType> *p = m_front;
        *e = p->data;
        m_front = p->next;
        delete p;
        return true;
    }

private:
    struct qnode<QElmType> *volatile m_front, *volatile m_rear;
};