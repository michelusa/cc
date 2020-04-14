#include <atomic>
#include <thread>
#include <iostream>
#include <string>
using namespace std;
atomic<unsigned> alloc {0};

template <typename T>
class LockFreeQueue {
    private:
        struct Node {
            Node(T* val) : value(val), next(nullptr) {}
            T* value;
            atomic<Node *> next;
        };

        Node *first, *last;
        atomic<bool> producerLock;
        atomic<bool> consumerLock;

    public:
        LockFreeQueue() {
            last = new Node(nullptr);
            first = last;
            producerLock = false;
            consumerLock = false;

        }
        ~LockFreeQueue() {
            while (first != nullptr) {
                Node *tmp = first;
                first = tmp->next;
                delete tmp;
            }
        }

        /* 
         * only one producer increase actually write at a time
         * just keep pointer to memory allocated
         * 
         */
        void put(  T* t) {
            Node *tmp  = new Node (t);

            while (producerLock.exchange(true)) {}

            last->next  = tmp;
            last = tmp;

            producerLock = false;
        }

        /*
         * only one consumer get each item and reclaim memory
         */
        bool get(T& result) {
           
               while (consumerLock.exchange(true)){}

            if (first->next != nullptr) {
                Node* oldFirst = first;
                first = first->next;
                alloc--;

                T* value = first->value;
                first->value = nullptr;  

                consumerLock = false;

                //copy value pointed at for use by consumer
                //before reclaiming payload memory
                //      and node allocated by a producer

                result = *value;
                delete value;
                delete oldFirst;

                return true;
            }
            consumerLock = false;
            return false;
        }



};
LockFreeQueue<string> lfq;
std::string SAMPLE {"hello, world: another: "};

//produce n items exactly
void publish (int n)
{
    for (int i = 0; i < n; ++i) {
        alloc++;
        lfq.put(new string(SAMPLE + to_string(i)));
    }

}


//consume n items exactly
void consume (unsigned n)
{
    unsigned received = 0;
    while (received < n) {
        string s;
        if(lfq.get(s)){
            ++received;
            ;//cout << s << endl;
        }
    }
}

auto main() -> int
{
    std::thread publisher (publish,  1000005);
    std::thread publisher2 (publish, 1000000);
    std::thread consumer (consume,   1000000);
    std::thread consumer2 (consume,        5);
    std::thread consumer3 (consume,   1000000);

    publisher.join();
    publisher2.join();
    consumer.join();
    consumer2.join();
    consumer3.join();
    cout << "alloc count left: " << alloc << endl;
    return 0;
}
