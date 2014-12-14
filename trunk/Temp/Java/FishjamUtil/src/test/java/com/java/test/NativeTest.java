package com.java.test;

/******************************************************************************************
 * 为了实现特殊的功能(如访问操作系统底层硬件设备等),必须通过 native 方式借助C语言来完成
 * 步骤:
 *   1.在Java程序中声明 native 方法，只有方法签名，没有实现。并编译生成对应的 .class 文件;
 *   2.用 javah 编译该 .class 文件，生成对应的 .h 文件;
 *   3.通过 C/C++ 实现该  .h 文件中的 native 方法(会 包含 jni.h);
 *   4.将 C/C++ 源文件编译成动态链接库文件
 *   5.Java 中用 System 或 Runtime 的 loadLibrary 加载该库文件，并调用 
******************************************************************************************/
public class NativeTest {

}
