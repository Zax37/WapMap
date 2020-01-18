#ifndef H_C_PARALLELLOOP
#define H_C_PARALLELLOOP

class cParallelCallback
{
 public:
  virtual void ParallelTrigger(){};
};

class cParallelLoop
{
 private:
  cParallelCallback * hCallback;
  float fDeltaTreshold;
 public:
  cParallelLoop();
  ~cParallelLoop();
  void SetCallback(cParallelCallback * h){ hCallback = h; };
  void SetFPS(int i);
  void Tick();
};

#endif // H_C_PARALLELLOOP
