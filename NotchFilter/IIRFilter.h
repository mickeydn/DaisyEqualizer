class IIRFilter {

	public:
		IIRFilter() {}
		~IIRFilter() {}

		void Init(float* a_coeffs, float* b_coeffs) 
		{
			for (int i = 0; i < 3; i++) 
			{
				a[i] = a_coeffs[i];
				b[i] = b_coeffs[i];
			}
			x1_ = x2_ = y1_ = y2_ = 0.0;
			bypass_ = true;
		}

		float Process(float x) 
		{
			if (bypass_) 
				return x;

			float res = b[0]*x + b[1]*x1_ + b[2]*x2_ - a[1]*y1_ - a[2]*y2_;

			y2_ = y1_;
			y1_ = res;
			x2_ = x1_;
			x1_ = x;

			return res;
		}

		void SetBypass(bool bypass) 
		{
			bypass_ = bypass;	
		}

	private:
		float a[3];
		float b[3];
		float x1_, x2_, y1_, y2_;
		bool bypass_;
};
