#include <iostream>

class MyClass{
    public:
        int myNum;

        MyClass(int x, int y, int z){
            printf("I have been created!\n");
            myNum = x;
            myNumPriv = y;
            myNumProtect = z;
        }

        ~MyClass(){
            printf("I have been destroyed!\n");
        }

        int getNum(){
            return myNumPriv;
        }
    private:
        int myNumPriv;
    protected:
        int myNumProtect;
};

class OtherClass : public MyClass{
    public:
        OtherClass(int a, int b, int c) : MyClass(a,b,c){}

        void func(){
            printf("myNum: %d\n", myNum);
            printf("myNumPriv: %d\n", getNum());
            printf("myNumProtect: %d\n", myNumProtect);
        }
};

int main(int argc, char *argv[])
{
    printf("Hello world!\n");

    OtherClass obj(4, 6, 9);
    obj.func();

    printf("Program running\n");
    return 0;
}
