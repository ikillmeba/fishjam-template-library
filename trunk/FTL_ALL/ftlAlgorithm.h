#ifndef FTL_ALGORITHM_H
#define FTL_ALGORITHM_H

/*******************************************************************************************************
* 各种算法
*   图像相似度算法的C#实现: http://www.doc88.com/p-34967244171.html
*     http://wenku.baidu.com/view/b80f3ef8770bf78a6529542b.html
*******************************************************************************************************/

/*******************************************************************************************************
* 压缩、解压缩 算法
*   LZW(Lempel Ziv Compression)算法 -- 
*     原理：提取原始图像数据中的不同图案，基于这些图案创建一个编译表，然后用编译表中的图案索引来
*            替代原始光栅数据中的相应图案，减少原始数据大小。类似调色板图像的实现原理。
*     编译表是根据原始图像数据动态创建的，解码时还要从已编码的数据中还原出原来的编译表。
*     数据流(CharStream,图像的光栅数据序列) 通过 编译表(String Table)计算，输出 编码流(CodeStream, 经过压缩运算的编码数据)
*******************************************************************************************************/

/*******************************************************************************************************
* 人眼视觉对RGB三基色的灵敏度各不相同，在量化过程中可以根据不同颜色对象使用不同特性的压扩变换方法，以取得最佳量化效果。
* 三种颜色对亮度影响系数为 R:G:B=0.299:0.597:0.114, 通常用相对影响系数 2:1:4
*
* 颜色量化算法(如 24位真彩转256色 ) -- 
*   流行色算法(Popularity Algorithm)
*     原理:对彩色图像中所有色彩出现的次数进行统计分析，从而选取频率最高的N(256)种颜色，为这N(256)种颜色建立调色板，
*          其余的颜色用最小距离原则映射到最临近的调色板颜色上。
*     优点:算法简单容易实现,对彩色数量小的图像变换效果好
*     缺点:一些出现频率较低，但对人眼视觉效果明显的信息将丢失(如高亮度斑点)
*   中位切分算法(Median Cut Algorithm)
*     原理:在RGB彩色空间中，R、G、B三基色对应于空间的三个坐标轴，将每一坐标轴都量化为0～255,
*           形成一个边长为256的彩色立方体，所有可能的颜色都对应于立方体内的点。取每个小立方体的中心点颜色。
*     优点:速度快
*     缺点:分隔与实际显示的图像无光，所以容易导致颜色表项的浪费。
*   八叉树颜色量化算法(Octree Color Quantization Algorithm) 
*     原理:将图像中使用的RGB颜色值分布到层状的八叉树中。八叉树的深度可达9层，即根节点层加上分别表示8位的R、G、B值的每一位的8层节点。
*          较低的节点层对应于较不重要的RGB值的位(右边的位)，为了提高效率和节省内存，可以去掉最低部的2 ~ 3层，对量化结果不会有太大影响。
*     特点:效率高、效果好
*     1.
*   聚类算法(Clustering Algorithm) -- 
*     原理:选择若干颜色作为聚类中心，按照距离准则使每种颜色向各个中心聚集，从而得到分类。
*     优点:对一般的颜色图像进行量化处理均能得到较好的量化效果
*     缺点：量化过程的空间开销较大，而且对初始类中心的选取非常重要，特别对色彩分布不均匀的图像，量化效果不够理想。
*            实现时需要不断聚合和调整聚类中心，计算量打，效率较低。
*******************************************************************************************************/

namespace FTL{
    
    
    class CColorQuantizzationBase{
    public:

    };

    /*******************************************************************************************************
    * a.giflib 中的量化算法?
    * 2.http://www.codeproject.com/Articles/109133/Octree-Color-Palette
    *
    * 八叉树颜色量化算法
    *   1.扫描图像的所有像素，将它们的数据累加到相应的节点中；遇到新的颜色则创建一个叶子节点，并将此像素的颜色数据存入其中。
    *   2.如果叶子节点数目大于目标调色板所要的颜色数，就将部分叶子节点合并到父节点中，并将父节点转化为叶子节点，
    *     在其中存放颜色分量的累加值及其像素出现的次数。同时删除原来被合并的叶子节点。
    *   3.所有像素处理结束，遍历八叉树，将叶子节点的各颜色值求平均值作为节点的颜色分量值读出并存入目标调色板。
    *   4.再次遍历所有像素，通过每个像素的颜色值与调色板中选中的256色运算，求得一个最接近像素颜色值的调色板颜色，把该像素换相应的调色板颜色索引
    *******************************************************************************************************/
    class COctreeColorQuantizzation : public CColorQuantizzationBase{

    };
}

#endif //FTL_ALGORITHM_H