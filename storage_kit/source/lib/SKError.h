//----------------------------------------------------------------------
// SKError.h
// Base error class thrown by all storage kit kernel interface functions
//----------------------------------------------------------------------
#ifndef _sk_error_h_
#define _sk_error_h_

const bool DO_DEBUG = true;

class SKError {
	public:
		SKError(int error, const char* errorMessage = "Undefined Storage Kit Error");
		virtual ~SKError();

		const int Error() const;
		const char* ErrorMessage() const;

	private:
		SKError(const SKError &ref);
			// Make the copy constructor private so only pointers may be
			// thrown (more efficient; less copy construction going on)
		SKError();
			// Make the default constructor private so some sort
			// or error code has to be specified
		void InitErrorMessage(const char* errorMessage);

		int fError;
		char *fErrorMessage;		
		
};

#endif

