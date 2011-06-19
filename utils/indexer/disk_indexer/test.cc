//#include <stdio.h>
//
//int main()
//{
//    printf("hello world.");
//    return 0;
//}
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <functional>　
using namespace std;

struct A 
{ 

    A()
    {

    };
    A(int c)
    {
        a=c;
    };

    int a; 
    int b;

};

struct compare : public binary_function <A*,A*,bool> 
{ 
    bool operator () (A const* const rhs, A const* const lhs) 
    { 
        return rhs->a < lhs->a; 
    } 
};

struct compare2 : public binary_function <A*,A*,bool> 
{ 
    bool operator () (A const* const rhs, A const* const lhs) 
    { 
        return rhs->a < (lhs->a)-2; 
    } 
};

vector < A* > avec;
struct show : public unary_function <A*,void>
{
    void operator () (A const* const rhs)
    {
        cout<<rhs->a<<" "<<rhs->b<<endl;
    }


};
void Show(A* p)
{
    cout<<p->a<<" "<<p->b<<endl;

}

int main()
{
    A * p1 = new A;
    p1->a=1;
    p1->b=5;

    A * p2 = new A;
    p2->a=3;
    p2->b=9;

    A * p3 = new A;
    p3->a=5;
    p3->b=5;


    A * p4 = new A;
    p4->a=6;
    p4->b=3;


    A * p5 = new A;
    p5->a=9;
    p5->b=5;

    A * p6 = new A;
    p6->a=8;
    p6->b=4;


    A * p7 = new A;
    p7->a=12;
    p7->b=1;

    A * p8 = new A;
    p8->a=20;
    p8->b=1;

    avec.push_back(p1);
    avec.push_back(p2);
    avec.push_back(p3);
    avec.push_back(p4);
    avec.push_back(p5);
    avec.push_back(p6);
    avec.push_back(p7);
    avec.push_back(p8);

    for_each(avec.begin(),avec.end(),Show);

    sort(avec.begin(),avec.end(),compare());

    for_each(avec.begin(),avec.end(),show());
    for_each(avec.begin(),avec.end(),Show);


    pair< vector <A*>::iterator, vector <A*>::iterator > it;

    it = equal_range(avec.begin(),avec.end(),&A(19),compare2());

    if (it.first != it.second)
    {
        cout<<"Sussecc!"<<endl;
    }
    else
    {
        cout<<"fail!"<<endl;
    }
    if (it.second != avec.end())
    {
        cout<<(*(it.first))->a<<"~"<<(*(it.second))->a<<endl;
    }
    else
    {
        cout<<(*(it.first))->a<<"~"<<it.second<<endl;

    }


    return 0;
}
