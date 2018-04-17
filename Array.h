#include <vector>
#include <iostream>
#include <utility>
#include <parallel/algorithm>
template<typename T>
std::ostream& operator<<(std::ostream &out, const std::vector<T> &arr)
{
	for(const auto& i:arr)
		out<<i<<" ";
	return out;
}
template<typename T>
struct Arr
{
	std::vector<size_t> strides;
	std::vector<T> mat;
	std::vector<size_t> dims;
	Arr(std::vector<size_t> &&x)
	{
		size_t s=x.size();
		strides.resize(s);
		dims.resize(s);
		auto s1=x[s-1];
		strides[s-1]=1;
		dims[s-1]=x[s-1];
		dims[0]=x[0];
		if(s>1)
		{
			size_t p=x[s-1];
			for(int i=s-2; i>=1; i--)
			{
				dims[i]=x[i];
				strides[i]=p;
				p*=x[i];
				s1*=x[i];
			}
			strides[0]=p;
			p*=x[0];
			s1*=x[0];
		}
		mat.resize(s1);
		std::cout<<"Dims= "<<dims<<"\n";
		std::cout<<"Strides= "<<strides<<"\n";
		std::cout<<"Size="<<s1<<"\n";
	}
	void reshape(std::vector<size_t> &&x)
	{
		size_t s=x.size();
		strides.resize(s);
		dims.resize(s);
		//size_t s1=1;
		strides[s-1]=1;
		dims[s-1]=x[s-1];
		dims[0]=x[0];
		if(s>1)
		{
			size_t p=x[s-1];
			for(int i=s-2; i>=1; i--)
			{
				dims[i]=x[i];
				strides[i]=p;
				p*=x[i];
			}
			strides[0]=p;
			p*=x[0];
			//s1*=p;
		}
		mat.resize(strides[0]*dims[s-1]);
		std::cout<<"Dims= "<<dims<<"\n";
		std::cout<<"Strides= "<<strides<<"\n";
	}
	inline T& operator[](const std::vector<size_t> &&indices)		//a bit slow...Please avoid loops with this
	{
		unsigned int sum=0;
		auto st=indices.size();
		for(int i=0; i<st; i++)
			sum+=strides[i]*indices[i];
		//cout<<"index="<<sum<<"\n";
		//cout<<"With "<<indices<<"\n";
		return mat[sum];
	}
	template<class Func>
	void iterate(int start, int length, int dimension, Func func)
	{
		for(int i=start, j=1; j<=length && i<dims[dimension]; i+=strides[dimension], j++)
			func(mat[i]);
	}
	template<class Func>
	void iterate_stride(int start, int length, int stride, Func func)
	{
		int n=mat.size();
		for(int i=start, j=1; j<=length && i<n; i+=stride, j++)
			func(mat[i]);
	}
	// template<class Comp>
	// void sort(vector<size_t> &&axes, Comp cmp)
	// {
	// 	for(auto& i:axes)
	// 	{
	// 		auto n=mat.size();
	// 		vector<T> tmp(n/strides[i]);
	// 		for(int j=0; j<n; j+=strides[i])
	// 			tmp[j/strides[i]]=mat[j];
	// 		//cout<<"Tmp= "<<tmp<<"\n";
	// 		std::sort(tmp.begin(), tmp.end(), [&cmp](const T &x, const T &y){return cmp(x, y);});
	// 		//cout<<"Tmp= "<<tmp<<"\n";
	// 		for(int j=0; j<n; j+=strides[i])
	// 			mat[j]=move(tmp[j/strides[i]]);
	// 		tmp.clear(); tmp.shrink_to_fit();
	// 	}
	// }
	template<class Func>
	void sort(std::vector<std::vector<size_t>> sizes, Func func)
	{
		auto sizes_n=sizes.size();
		for(size_t i=0; i<sizes_n; i++)
		{
			auto a1=sizes[i][0];
			auto a2=sizes[i][1];
			auto a3=sizes[i][2];
			std::vector<T> arr(a2); int index=0;
			for(size_t i1=a1, j=1; j<=a2; i1+=a3, j++)
				arr[index++]=mat[i1];
			__gnu_parallel::sort(arr.begin(), arr.end(), func);
			index=0;
			for(size_t i1=a1, j=1; j<=a2; i1+=a3, j++)
				mat[i1]=arr[index++];
		}
	}
	void swapaxes(int x, int y)
	{
		std::swap(strides[x], strides[y]);
		std::swap(dims[x], dims[y]);
	}
	Arr<T> span(std::vector<std::pair<size_t, size_t>> sizes)               //crying bitterly for template metaprogramming....Please HELP!!!!!!
	{
		auto tmp=sizes.size();
		if(tmp==1)
		{
			auto a1=sizes[0].first, a2=sizes[0].second;
			Arr<T> arr=Arr<T>({a2-a1+1}); int index=0;
			for(int i=a1; i<=a2; i++)
				arr.mat[index++]=mat[strides[0]*i];
			return arr;
		}
		else if(tmp==2)
		{
			auto a1=sizes[0].first, a2=sizes[0].second;
			auto b1=sizes[1].first, b2=sizes[1].second;
			Arr<T> arr=Arr<T>({a2-a1+1, b2-b1+1}); int index=0;
			for(int i=a1; i<=a2; i++)
				for(int j=b1; j<=b2; j++)
					arr.mat[index++]=mat[strides[0]*i+strides[1]*j];
			return arr;
		}
		else if(tmp==3)
		{
			auto a1=sizes[0].first, a2=sizes[0].second;
			auto b1=sizes[1].first, b2=sizes[1].second;
			auto c1=sizes[1].first, c2=sizes[1].second;
			Arr<T> arr=Arr<T>({a2-a1+1, b2-b1+1, c2-c1+1}); int index=0;
			for(int i=a1; i<=a2; i++)
				for(int j=b1; j<=b2; j++)
					for(int k=c1; k<=c2; k++)
						arr.mat[index++]=mat[strides[0]*i+strides[1]*j+strides[2]*k];
			return arr;
		}
	}
	Arr<T> span(std::vector<std::vector<size_t>> sizes, std::vector<size_t> shape)
	{
		Arr<T> arr=Arr<T>(std::move(shape));
		auto n=sizes.size();
		size_t index=0;
		for(int i=0; i<n; i++)
		{
			auto a1=sizes[i][0];
			auto a2=sizes[i][1];
			auto a3=sizes[i][2];
			for(size_t j=1, k=a1; j<=a2; j++, k+=a3)
				arr.mat[index++]=mat[k];
		}
		return arr;
	}
	std::vector<T*> at(std::vector<size_t> pos)
	{
		std::vector<T*> arr1;
		size_t n=pos.size(), index=0;
		for(size_t i=0; i<n; i++)
			index+=pos[i]*strides[i];
		n=index+strides[n-1];
		while(index<n)
			arr1.emplace_back(&mat[index++]);
		return arr1;
	}
};