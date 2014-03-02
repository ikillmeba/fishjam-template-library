package com.fishjam.study.java;
/****************************************************************************************
 * Java性能优化：
 *   1.尽量减少临时对象的使用
 *   2.对象不用时最好显式置为Null
 *   3.尽量使用StringBuffer,而不用String来累加字符串
 *   4.能用基本类型如Int,Long,就不用Integer,Long对象
 *   5.尽量少用静态对象变量--静态变量属于全局变量,不会被GC回收,它们会一直占用内存
 *   6.分散对象创建或删除的时间
 * 
 * 垃圾回收(Garbage Collector，GC)
 *   1.只能回收new出来的内存，且释放顺序是随机的(不是说越早成为垃圾越早释放)
 *   2.GC会在回收内存的同时，重排heap中的所有对象(包括正在使用的)，使其紧密排列，避免出现C/C++中的内存碎片问题.
 *   3.Java 中有 protected void finalize 方法，会在第一次垃圾回收时自动调用，常释放句柄、Socket等，然后在第二次垃圾回收时回收内存。
 *     第一次GC -> finalize -> 下一次GC释放内存
 *     ★如果override了finalize方法，千万记得调用 super.finalize ，否则base类的finalize将不会被调用。★
 *     由于是保护型的，不能被直接调用(通过重写成public可以直接调用？在finally中调用？有没有类似C#的using？)
 *     注意：★垃圾回收不等于析构★，最好手动执行必要的释放函数(finalize)来关闭相关的资源(文件句柄的)
 *   4.两个条件会触发主GC：
 *     a.应用程序空闲时；
 *     b.堆内存不足时，
 *     若三次GC后内存仍然不足，则JVM将报“out of memory”的错误,Java应用将停止。
 *   5.GC的算法不采用引用计数的方式(会出现有交叉引用的垃圾无法释放的情况)，而采用从stack或静态开始找所有能找到的对象，并将没有被找到的对象作为垃圾清除
 *
 * ref中的类在持有大量内存对象时格外有用--可以在保持引用的同时进行垃圾回收(类似Cache？但再次访问怎么办？) 
 * PhantomReference --
 * SoftReference -- 多用来实现cache机制 WeakReference -- 一般用来防止内存泄漏，要保证内存被VM回收?
 *   
 * Java 执行效能的改善(执行速度慢) 
 *   1.JDK1.3引入了 hotspot 技术，可以大幅改善执行效能 
 *   2.JIT(Just-In-Time)编译器 -- 在运行时按需编译，可以减少编译时间和运行时的程序大小，但初次运行时较慢?
 *   3.GC时，将回收回来的内存进行重排(Compact)，同时更改原来引用指向的地址；使得以后再次分配内存时，能简单的连续分配；
 *
 *
****************************************************************************************/

public class PerformanceStudy {

}
