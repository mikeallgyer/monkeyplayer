#pragma once


#include <string>
#include <vector>

#include "DatabaseManager.h"

#include <msclr/marshal.h>
using namespace msclr::interop;

/*using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
*/
//#include <msclr/marshal.h>
//using namespace msclr::interop;


namespace MonkeyPlayer {

	/// <summary>
	/// Summary for SavePlaylistForm
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class SavePlaylistForm : public System::Windows::Forms::Form
	{
	public:
		SavePlaylistForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			mFinished = false;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SavePlaylistForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::TextBox^  mNameTxt;
	private: System::Windows::Forms::Button^  mOkBtn;
	private: System::Windows::Forms::Button^  mCancelBtn;

	protected: 

	protected: 

	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;
		bool mFinished;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->mNameTxt = (gcnew System::Windows::Forms::TextBox());
			this->mOkBtn = (gcnew System::Windows::Forms::Button());
			this->mCancelBtn = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(12, 18);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(35, 13);
			this->label1->TabIndex = 0;
			this->label1->Text = L"Name";
			// 
			// mNameTxt
			// 
			this->mNameTxt->Location = System::Drawing::Point(53, 15);
			this->mNameTxt->Name = L"mNameTxt";
			this->mNameTxt->Size = System::Drawing::Size(298, 20);
			this->mNameTxt->TabIndex = 1;
			// 
			// mOkBtn
			// 
			this->mOkBtn->Location = System::Drawing::Point(276, 52);
			this->mOkBtn->Name = L"mOkBtn";
			this->mOkBtn->Size = System::Drawing::Size(75, 23);
			this->mOkBtn->TabIndex = 2;
			this->mOkBtn->Text = L"&OK";
			this->mOkBtn->UseVisualStyleBackColor = true;
			this->mOkBtn->Click += gcnew System::EventHandler(this, &SavePlaylistForm::mOkBtn_Click);
			// 
			// mCancelBtn
			// 
			this->mCancelBtn->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->mCancelBtn->Location = System::Drawing::Point(195, 52);
			this->mCancelBtn->Name = L"mCancelBtn";
			this->mCancelBtn->Size = System::Drawing::Size(75, 23);
			this->mCancelBtn->TabIndex = 3;
			this->mCancelBtn->Text = L"C&ancel";
			this->mCancelBtn->UseVisualStyleBackColor = true;
			this->mCancelBtn->Click += gcnew System::EventHandler(this, &SavePlaylistForm::mCancelBtn_Click);
			// 
			// SavePlaylistForm
			// 
			this->AcceptButton = this->mOkBtn;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->mCancelBtn;
			this->ClientSize = System::Drawing::Size(365, 85);
			this->Controls->Add(this->mCancelBtn);
			this->Controls->Add(this->mOkBtn);
			this->Controls->Add(this->mNameTxt);
			this->Controls->Add(this->label1);
			this->Name = L"SavePlaylistForm";
			this->Text = L"Save Playlist";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: 
		System::Void mOkBtn_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 if (mNameTxt->Text->Length > 0)
			 {
				 mFinished = true;

				this->Close();
			 }
		 }

		public:
			std::string getPlaylistName()
			{
				marshal_context ^ context = gcnew marshal_context();
				const char* str4 = context->marshal_as<const char*>(mNameTxt->Text);
				std::string retVal = str4;
				return retVal;
			}
			void setPlaylistName(std::string name)
			{
				mNameTxt->Text = gcnew System::String(name.c_str());
			}
			bool getIsFinished()
			{
				return mFinished;
			}
private: 
	System::Void mCancelBtn_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		mFinished = false;
		this->Close();
	}
};
}
