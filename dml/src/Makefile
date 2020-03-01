#TARGET:= hello server client #我们的目标文件
Target1= hello 
Target2= server
Target3= client

TARGET:= $(Target1) $(Target2) $(Target3) #我们的目标文件

CC:= g++ #编译工具
#SUB_MODULE_DIR=submodule#子模块目录

#EXTRA_LIB:=libmytest.so #使用到的动态连接库

OBJECTS:= server.o client.o main.o #依赖的目标文件
#HEAD:= a.h b.h together.h  #依赖的头文件
#HEAD+= d.h  #添加依赖的头文件

#HIDE?=@  这里，添加@后，就不会打印所有的编译日志信息了
#一个例子解释说明一下使用的技巧
# target: relay1 relay2 relay3 ... relay*
#$< --> the first replay, i.e., relay1
#$^ --> all the relays, i.e., relay1 relay2 relay3 ... relay*
#$@ --> the output object, i.e., target

DEBUGINFO:=-Wall -g -O0 #调试选项
CXXFLAGS+= -Os $(DEBUGINFO) -I$(SUB_MODULE_DIR)  #编译选项
#LINKFLAGS:=-L./ -lmytest -Wl,-rpath,$(PWD) #链接选项

all:clean $(TARGET)  #默认的make构建目标
	@echo "Make $(TARGET) Done"

%.o:%.cpp $(HEAD) #编译生成中间目标文件的依赖和规则，这里使用的是隐式推导规则
	$(HIDE)$(CC) -c $(CXXFLAGS) $< -o $@

$(Target1):#链接生成中间目标文件的依赖和规则
	$(HIDE)$(CC) main.cc -o $@ -std=c++11 -Os

$(Target2):  #链接生成中间目标文件的依赖和规则
	$(HIDE)$(CC) server.cc -o $@ -std=c++11 -Os

$(Target3): #链接生成中间目标文件的依赖和规则
	$(HIDE)$(CC) client.cc -o $@ -std=c++11 -Os

run:$(TARGET) #测试用例的规则
	./$(Target1)

.PHONY:clean
clean: #清理规则
	rm -f $(TARGET) $(OBJECTS)
	@echo "Clean done"