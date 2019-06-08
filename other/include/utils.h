# ifndef UTILS_H
# define UTILS_H

#include "includes.h"
namespace INTERPRETER {
	namespace TMP {
		namespace BASICTYPE {

			// Int type
			template<class T, T v = 0>
			struct Int {
				static constexpr T value = v;
				using value_type = T; 
				using type = Int;
				constexpr operator value_type() const noexcept { return value; }
				constexpr value_type operator()() const noexcept { return value; }
			};
			using Int_0 = Int<int, 0>;
			using Int_1 = Int<int, 1>;


			// Bool type
			template<bool B> using Bool = Int<bool, B>;
			using TrueType = Bool<true>;
			using FalseType = Bool<false>;

			template<typename Exp>
			struct _Eval {
				enum { value = Exp::result::value };
				using result = _Eval<Exp>;
			};

		}

		namespace HELPER {
			using BASICTYPE::Bool;
			using BASICTYPE::Int;
			using BASICTYPE::TrueType;
			using BASICTYPE::FalseType;

			template<class T1, class T2> struct type_equal { using type = FalseType; };
			template<class T>
			struct type_equal<T, T> {
				using type = TrueType;
			};
			template<class T1, class T2> using type_equal_t = typename type_equal<T1, T2>::type;


			template<class T1, class T2> struct type_not_equal { using type = TrueType; };
			template<class T>
			struct type_not_equal<T, T> {
				using result = FalseType;
			};
			template<class T1, class T2> using type_not_equal_t = typename type_not_equal<T1, T2>::type;


			template<class B, class T = void> struct enable_if {};
			template<class T>
			struct enable_if<TrueType, T> { using type = T; };
			template<class B, class T> using enable_if_t = typename enable_if<B, T>::type;


			template<class B, class T1, class T2> struct select {};
			template<class T1, class T2>
			struct select<TrueType, T1, T2> { using type = T1; };
			template<class T1, class T2>
			struct select<FalseType, T1, T2> { using type = T2; };
			template<class B, class T1, class T2> using select_t = typename select<B, T1, T2>::type;


			struct EmptyType;
			template<class Head, class Tail> struct cons {};
			template<class Head, class... Tail>
			struct TypeList {
				using value = cons< Head, typename TypeList<Tail...>::value >;
			};
			template<class Head>
			struct TypeList<Head> {
				using value = cons<Head, EmptyType>;
			};

			template<class List> struct car {};
			template<class Head, class Tail> struct car<cons<Head, Tail>> {
				using value = Head;
			};

			template<class List> struct cdr {};
			template<class Head, class Tail> struct cdr<cons<Head, Tail>> {
				using value = Tail;
			};
		}

		namespace OPERATIONS {
			using namespace BASICTYPE;

			template<typename T1, typename T2> struct _SumExp;
			template<typename T1, typename T2> struct _SubExp;
			template<typename T1, typename T2> struct _MulExp;
			template<typename T1, typename T2> struct _DivExp;

			template<int T1, int T2>
			struct _SumExp<Int<int, T1>, Int<int, T2>> {
				using result = Int<int, T1 + T2>;
			};

			template<int T1, int T2>
			struct _SubExp<Int<int, T1>, Int<int, T2>> {
				using result = Int<int, T1 - T2>;
			};

			template<int T1, int T2>
			struct _MulExp<Int<int, T1>, Int<int, T2>> {
				using result = Int<int, T1* T2>;
			};

			template<int T1, int T2>
			struct _DivExp<Int<int, T1>, Int<int, T2>> {
				using result = Int<int, T1 / T2>;
			};
			
			template<typename Condition, typename Then, typename Else> struct IF { };
			template<typename Then, typename Else>
			struct IF<TrueType, Then, Else> {
				using result = Then;
			};
			template<typename Then, typename Else>
			struct IF<FalseType, Then, Else> {
				using result = Else;
			};

			template<typename T1, typename T2> struct _Equal { using result = FalseType; };
			template<typename T1, typename T2> struct _Less { using result = FalseType; };
			template<typename T1, typename T2> struct _Great { using result = FalseType; };
			template<typename T1, typename T2> struct _And;
			template<typename T1, typename T2> struct _Or;
			template<typename T1, typename T2> struct _Less;
			template<typename T> struct _Not;

			template<typename T>
			struct _Equal<T, T> {
				using result = TrueType;
			};

			template<int T1, int T2>
			struct _Less<Int<int, T1>, Int<int, T2>> {
				using result =
					typename IF < Bool<Int<int, T1>::value < Int<int, T2>::value>,
					TrueType,
					FalseType
					>::result;
			};

			template<typename Statement>
			struct _Skip {
				using result = Statement;
			};

			template<typename T, T v>
			struct _Assign {
				using result = typename Int<T, v>::result;
			};

			template<bool B1, bool B2>
			struct _And<Bool<B1>, Bool<B2>> {
				using result = Bool<B1&& B2>;
			};

			template<bool B1, bool B2>
			struct _Or<Bool<B1>, Bool<B2>> {
				using result = Bool<B1 || B2>;
			};

			template<bool B>
			struct _Not<Bool<B>> {
				using result = Bool<!B>;
			};

			template<typename T1, typename T2>
			using SumExp = typename _SumExp<T1, T2>::result;

			template<typename T1, typename T2>
			using SubExp = typename _SubExp<T1, T2>::result;

			template<typename T1, typename T2>
			using MulExp = typename _MulExp<T1, T2>::result;

			template<typename T1, typename T2>
			using DivExp = typename _DivExp<T1, T2>::result;

			template<typename T1, typename T2>
			using And = typename _And<T1, T2>::result;

			template<typename T1, typename T2>
			using Or = typename _Or<T1, T2>::result;

			template<typename T1, typename T2>
			using Equal = typename _Equal<T1, T2>::result;

			template<typename T1, typename T2>
			using Less = typename _Less<T1, T2>::result;

			template<typename T1, typename T2>
			using Great = typename _Less<T2, T1>::result;

			template<typename T1, typename T2>
			using LessEqual = Or<typename _Less<T1, T2>::result, typename _Equal<T1, T2>::result>;

			template<typename T1, typename T2>
			using GreatEqual = Or<typename _Great<T1, T2>::result, typename _Equal<T1, T2>::result>;

			template<typename Statement>
			using Skip = typename _Skip<Statement>::result;

			template<typename T, T v>
			using Assign = typename _Assign<T, v>::result;

			template<typename T>
			using Not = typename _Not<T>::result;

			template<typename T>
			using eval = typename _Eval<T>::result;

			template<typename Statement>
			struct WHILESTOP {
				using result = Statement;
			};
			template<template<typename> class Condition, typename Statement>
			struct WHILE {
				using result = typename
					IF<Bool<Condition<Statement>::result>,
					WHILE<Condition, typename Statement::Next>,
					WHILESTOP<Statement>
					>::result::result;
			};
		}

		namespace KEYWORDS {
			using namespace BASICTYPE;


		}
	}
	namespace UTILS {
		string type_to_str(int type);

		class Print {
		public:
			template<template<class, class...> class ContainerType, class ValueType, class... Args>
			static void print(const ContainerType<ValueType, Args...>& c){
				for (const auto& v : c) {
					cout << v << " ";
				}
				cout << endl;
			}
		};

		template<typename T>
		class Matrix3D {
		private:
			int rows;
			int cols;
			int high;
			vector<vector<vector<T>>>  Mat;
		public:
			Matrix3D (int rows, int cols, int high);
			Matrix3D operator -(const Matrix3D& other)const;
			Matrix3D operator +(const Matrix3D& other)const;
			Matrix3D operator *(const Matrix3D& other)const;
		};

		template <typename T>
		Matrix3D<T>::Matrix3D(int rows, int cols, int high) : rows(rows), cols(cols), high(high){
			vector<vector<vector<T>>> A (rows, vector<vector<double> >(cols, vector <double>(high, 0)));
			int cnt = 0;
			for (int i = 0; i < rows; i++){
				for (int j = 0; j < cols; j++){
					for (int k = 0; k < high; ++k){
						A[k][j][i] += cnt * 1.0;
						++cnt;
					}
				}
			}			
			Mat = A;
		}

		template <typename T>
		Matrix3D<T>  Matrix3D<T>::operator +(const Matrix3D& other)const {  //add 2 matrix
			Matrix3D<T> temp(rows, cols, high);  
			for(int i = 0; i < rows; i++)
				for(int j = 0; j < cols; j++)
					for (int k = 0; k < high; k++)
						temp.Mat[k][j][i] += other.Mat[k][j][i] + Mat[k][j][i];
			return temp; 
		}

		template <typename T>
		Matrix3D<T>  Matrix3D<T>::operator *(const Matrix3D &other)const   { //multiplay matrix on the right
			Matrix3D<T> temp(rows, cols, high); 
			for(int i = 0; i < rows; i++)
				for(int j = 0; j < cols; j++)
					for (int k = 0; k < high; k++)
							temp.Mat[k][j][i] += other.Mat[k][j][i] * Mat[k][j][i];
			return temp;          
		}


		template <typename T>
		Matrix3D<T>  Matrix3D<T>::operator -(const Matrix3D &other)const { //matrix subtraction {
			Matrix3D<T> temp(rows, cols, high);  
			for(int i = 0; i < rows; i++)
				for(int j = 0; j < cols; j++)
					for (int k = 0; k < high; k++)
						temp.Mat[k][j][i] -= other.Mat[k][j][i] + Mat[k][j][i];
			return temp; 
		}
	}
}


#endif# 
