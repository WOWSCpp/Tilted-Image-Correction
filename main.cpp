#include<iostream>
#include<fstream>
#include<string>
#include<cstdlib>
#include<vector>
using namespace std;
const int Num = 9;

/*****************************以下为线性表类模板定义***************************************/
template<typename Type> class TList;           //线性表类模板声明
template<typename Type> class TNode            //线性表结点类模板定义
{
	friend class TList<Type>;      //设置class TList为TNode的友元,允许访问TNode的私有成员
								   //因友元TList仅需访问private成员，故省略默认构造函数，复制构造函数，赋值操作符等,由编译器自动生成。
private:
	TNode<Type>* Next;          //线性表next指针
	Type Data;				   //线性表成员
};

template<typename Type> class TList               //线性表类模板定义
{
public:
	TList() :Last(nullptr), First(nullptr), Length(0) {} //构造函数。在构造函数本体前初始化，效率更高
	const int Getlen() const { return Length; } //返回线性表长度，只读
	int Append(const Type& T);              //从表尾加入结点
	int Insert(const Type& T, int k);        //插入结点，k为插入位置
	Type GetData(int i);                    //返回结点数据成员
	void SetData(const Type& T, int k);      //设置结点数据成员
private:
	TNode<Type>* First, * Last;               //TNode<Type>类型的线性表首、尾指针
	int Length;                             //线性表长度
};

template<typename Type> inline int TList<Type>::Append(const Type& T)  //向表尾插入节点
{
	Insert(T, Length);     //调用自定义Insert函数像表尾插入节点
	return 1;
}

template<typename Type> inline int TList<Type>::Insert(const Type& T, int k)  //插入节点
{
	TNode<Type>* p = new TNode<Type>;  //获取内存
	p->Data = T;
	if (First)
	{
		if (k <= 0)
		{
			p->Next = First;
			First = p;
		}
		if (k > Length - 1)
		{
			Last->Next = p;
			Last = Last->Next;
			Last->Next = nullptr;
		}
		if (k > 0 && k < Length)
		{
			k--;
			TNode<Type>* q = First;
			while (k-- > 0)
				q = q->Next;
			p->Next = q->Next;
			q->Next = p;
		}
	}
	else
	{
		First = Last = p;
		First->Next = Last->Next = nullptr;
	}
	Length++;
	return 1;
}

template<typename Type> inline Type TList<Type>::GetData(int k)  //返回节点数据
{
	TNode<Type>* p = First;
	while (k-- > 0)
		p = p->Next;
	return p->Data;
}

template<typename Type> inline void TList<Type>::SetData(const Type& T, int k)  //设置节点数据
{
	TNode<Type>* p = First;
	while (k-- > 0)
		p = p->Next;
	p->Data = T;
}
/*********************以上为线性表类模板定义*********************/
/******************以下为八数码问题搜索算法基类定义**************/
/********************此基类可派生多种搜索算法********************/
class Eight
{
public:
	Eight() {} //默认构造函数
	Eight(const char* fname);     //自定义构造函数，派生类中为打开第一个文档
	virtual void Search() = 0; //纯虚的搜索函数 使得此基类为抽象类。不能创造此基类的实体
	virtual ~Eight() {}
private:
	static int dis[4];//右、下、左、上移动的代价
	static int total; //空格移动次数
protected:
	vector<int> start;      //初始九宫图
	int last, spac;       //spac为空格位置
	vector<int> end;        //目标九宫格
	void print();		 //打印结果
	bool operator==(const Eight& T);//重载等于操作符
	bool Extend(int i); //判断是否还能移动
};

int Eight::dis[4] = { 1,3,-1,-3 };  //空格的位置向  右、上、左、下移动后的位置。
int Eight::total = 0;

inline Eight::Eight(const char* fname)  //从文档写入数据
{
	start.resize(9);
	end.resize(9);
	ifstream fin;  //fin为文件写出流的名字。
	fin.open(fname);//打开文档
	if (!fin)		//如果打开失败
	{
		cout << "不能打开数据文件!" << endl;
		return;
	}
	int i;
	/****根据派生类定义*******/
	for (i = 0; i < Num;)
		fin >> start[i++];//从第一个文档第1行读入数据
	fin >> spac;		//从第一个文档第2行读入数据，即空格位置
	for (i = 0; i < Num;)
		fin >> end[i++];//从第一个文档第3行读入数据
	fin.close();
	last = -1;
	total = 0;  //每次尝试计算前给移动次数赋初值
}
inline void Eight::print()
{
	cout << "第" << total++ << "次交换" << endl;
	cout << "  " << start[0] << "  " << start[1] << "  " << start[2] << endl
		<< "                                 " << endl
		<< "  " << start[3] << "  " << start[4] << "  " << start[5] << endl
		<< "                                 " << endl
		<< "  " << start[6] << "  " << start[7] << "  " << start[8] << endl;  //打印每步计算的结果
}

inline bool Eight::operator==(const Eight & T)  //重载等于操作符
{
	for (int i = 0; i < Num;)
		if (T.start[i] != start[i++])
			return 0;
	return 1;
}

inline bool Eight::Extend(int i)  //i代表四个方向，0,1,2,3分别代表右，下，左，上。
{
	if (i == 0 && spac % 3 == 2 ||  //如果已经在最右边并试图往右边移动
		i == 1 && spac > 5 ||  //如果已经在最上边并试图往上边移动
		i == 2 && spac % 3 == 0 || //如果已经在最左边并试图往左边移动
		i == 3 && spac < 3)      //如果已经在最左边并试图往左边移动
		return 0;			 //返回0则移动失败
	int temp = spac;    //记录此时空格的位置
	spac += dis[i];     //计算空格朝某个方向移动后的新位置。
	start[temp] = start[spac];//交换空格处的值(0)和新位置的值
	start[spac] = 0;      //交换后空格处重新置零
	return 1;       //返回1则移动成功
}
/****************以上为八数码问题搜索算法基类定义******************/
/****************以下为八数码A*搜索算法派生类定义******************/
class Astar :virtual public Eight
{
public:
	Astar() {}                      //默认构造函数
	Astar(const char* fname1, const char* fname2);   //自定义构造函数，用以打开文档
	virtual void Search();  //A*搜索算法
	void Print_public();//打印公有的输出
private:
	int f, g, h, i;                             //估价函数     f=g+h
	int r[Num];                              //存储状态中各个数字位置的辅助数组
	int s[Num];                              //存储目标状态中各个数字位置的辅助数组
	int e[Num * Num];                          //存储各个数字相对距离的辅助数组
	void Printl(TList<Astar> L);             //输出搜索路径
	int Expend(int i);                       //A*算法的状态扩展函数
	int Calcuf();                            //计算估价函数
	void Sort(TList<Astar>& L, int k);		 //将新扩展结点按f从小到大顺序插入待扩展结点队列
	int Repeat(TList<Astar>& L);			 //检查结点是否重复
};
inline void Astar::Print_public()
{
	cout << "为方便起见，本程序输出结果中都用0来代替空格" << endl;
}
inline Astar::Astar(const char* fname1, const char* fname2) :Eight(fname1) // 基类打开fname1
{
	for (int i = 0; i < Num;)
	{
		r[start[i]] = i;             //存储初始状态每个数字的位置
		s[end[i]] = i++;           //存储目标状态每个数字的位置
	}
	ifstream fin;
	fin.open(fname2);          //打开数据文件，派生类打开fname2
	if (!fin)
	{
		cout << "不能打开数据文件!" << endl;
		return;
	}
	for (i = 0; i < Num * Num; i++)   //读入各个数字相对距离值
		fin >> e[i];
	fin.close();
	f = g = h = 0;       //估价函数初始值
}

inline void Astar::Printl(TList<Astar> L) //输出函数
{
	Astar T = *this;
	if (T.last == -1)
		return;
	else
	{
		T = L.GetData(T.last);
		T.Printl(L);
		T.print();
	}
}

inline int Astar::Expend(int i)      //节点扩展函数
{
	if (Extend(i))              //结点可扩展
	{
		int temp = r[start[r[0]]];   //改变状态后数字位置变化，存储改变后的位置
		r[start[r[0]]] = r[0];
		r[0] = temp;
		return 1;
	}
	return 0;
}

inline int Astar::Calcuf()        //计算估价函数f
{
	h = 0;
	for (int i = 0; i < Num; i++)  //计算估价函数的h
		h += e[Num * r[i] + s[i]];
	return ++g + h;           //f = g+h
}

inline void Astar::Sort(TList<Astar> & L, int k)//排序函数，将新扩展结点按f从小到大顺序插入待扩展结点队列
{
	int n = L.Getlen();      //获取线性表长度
	for (int i = k + 1; i < n; i++) //从第K个节点开始遍历
	{
		Astar T = L.GetData(i);//比较估价函数大小
		if (this->f <= T.f)
			break;
	}
	L.Insert(*this, i);
}

inline int Astar::Repeat(TList<Astar> & L)//检查节点是否重复函数
{
	int n = L.Getlen();
	for (int i = 0; i < n; i++)
		if (L.GetData(i) == *this)
			break;
	return i;
}

inline void Astar::Search()
{
	const std::size_t N = 150;
	vector<vector<vector<double> > > A (N,vector<vector<double> >(N,vector <double>(N,0)));
	vector<vector<vector<double> > > B (N,vector<vector<double> >(N,vector <double>(N,0)));
	int ind;

	for (int i = 10; i < N; i++) {
		for (int j = 10; j < N; j++) {
			for (int k = 0; k < N; ++k){
				A[k][j][i] += ind;
				B[k][j][i] += ind;
			}
		}
	}

	vector<double> D;

	for (int i = 10; i < N; i++)
		for (int j = 10; j < N; ++j)
			for (int k = 0; k < N; ++k)
				if (rand() % 3 == 0)
					D.emplace_back(A[k][j][i] + B[k][j][i]);
}
/**********************以上为八数码派生类定义************************/
int main()
{
	Astar aStar("eight.txt", "eight_dis.txt");
	aStar.Search();
	return 0;
}
