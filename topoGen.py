import networkx as nx
import random
import math

def construct_overlay_network(N, p):
    G = nx.Graph()  # 创建一个空图

    # 添加起始节点n0
    G.add_node('n0')

    # 创建临时的节点列表
    existing_nodes = list(G.nodes)

    # 迭代从n1到nN-1的其他节点
    for i in range(1, N):
        node = f'n{i}'  # 生成节点名称

        # 连接新节点到现有节点的概率为p
        for existing_node in existing_nodes:
            if random.random() <= p:
                G.add_edge(existing_node, node)

        # 添加新节点到图中
        G.add_node(node)

        # 更新临时的节点列表
        existing_nodes.append(node)

    return G

# 示例用法
N = 10 # 节点数量（包括起始节点
p=1 #全连通图
print('Probability of establishing a link: ',p)
connected=False
while connected is False :
    overlay_network = construct_overlay_network(N, p)
    connected = nx.is_connected(overlay_network)
    # flag=len(overlay_network.edges)

print('节点数量：',N,' 边数量：',len(overlay_network.edges))
for i in overlay_network.edges:
    print(i[0][1:],"  ",i[1][1:],"    0.01       0     13333")

