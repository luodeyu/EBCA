package node;

//
// Application model to generate traffic for the network.
//
simple AppOld like IApp
{
    parameters:
        int address;  // local node address
        int leader;//负责整理交易并打包成块的leader节点
        int batchSize;
        int quorum;
        int N;
        string destAddresses;  // destination addresses
        string router;
        volatile double sendIaTime @unit(s);//  time between generating requests
        // volatile double sendInterval @unit(s) = default(exponential(100ms));//官方写法,供语法参考
        int packageLength @unit(byte);  // length of one message
        int voteLength @unit(byte);  // length of one vote
        @display("i=block/browser");
        
        @signal[endToEndDelay](type="simtime_t");
        @statistic[endToEndDelay](title="end-to-end delay of arrived requests";unit=s;record=vector,mean;interpolationmode=none);

        @signal[TxQueueLen](type="long");
        @statistic[TxQueueLen](title="TxQueueLen";record=vector,mean;interpolationmode=none);
        
        @signal[Txcount](type="long");
        @statistic[Txcount](title="Txcount";record=vector?,sum;interpolationmode=none);
    gates:
        input in;
        output out;
}