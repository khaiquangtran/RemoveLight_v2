#ifndef PACKAGE_H
#define PACKAGE_H

class Package
{
public:
  Package(int data[], int size)
  {
    mData = new int[size];
    mSize = size;
    for (int i = 0; i < size; i++)
      mData[i] = data[i];
  }

  ~Package()
  {
    delete[] mData;
  }

  int *getPackage()
  {
    return mData;
  }

  int getSize()
  {
    return mSize;
  }

private:
  int *mData;
  int mSize;
};

#endif // PACKAGE_H