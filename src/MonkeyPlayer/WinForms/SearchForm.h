#pragma once

#include <string>
#include <vector>

#include "DatabaseManager.h"

#include <msclr/marshal.h>
using namespace msclr::interop;
/*
using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
*/

namespace MonkeyPlayer
{

	/// <summary>
	/// Summary for SearchForm
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class SearchForm : public System::Windows::Forms::Form
	{
		ref class ListBoxItem : public System::Object
		{
		public:
			int mId;
			System::String^ mString;
			virtual System::String^ ToString() override { return mString; }
		};
	public:
		SearchForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			mSelectedTrackID = -1;
			mPlayBtn->Enabled = false;
			mGoToBtn->Enabled = false;
			mDoPlay = false;
			mDoGoTo = false;
			mIsOpen = false;
		}

		int getSelectedTrackID()
		{
			return mSelectedTrackID;
		}

		bool doPlay()
		{
			return mDoPlay;
		}
		
		bool doGoTo()
		{
			return mDoGoTo;
		}

		bool isOpen()
		{
			return mIsOpen;
		}

		void reset()
		{
			mDoPlay = false;
			mDoGoTo = false;
			mIsOpen = false;
		}

		static void openSearch()
		{
			if (mInstance->isOpen())
			{
				mInstance->Close();
				delete mInstance;
			}
			mInstance = gcnew SearchForm();

			mInstance->ShowDialog();
		}
		
		static SearchForm^ instance()
		{
			return mInstance;
		}

		void threadClose()
		{
			this->Invoke(gcnew System::Windows::Forms::MethodInvoker(this, &SearchForm::Close));
		}
	protected:
		int mSelectedTrackID;
		bool mDoPlay;
		bool mDoGoTo;
		bool mIsOpen;

		static SearchForm^ mInstance = gcnew SearchForm();

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SearchForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Label^  label1;
	protected: 
	private: System::Windows::Forms::TextBox^  mSearchTxt;
	private: System::Windows::Forms::ListBox^  mTrackList;

	private: System::Windows::Forms::Button^  mPlayBtn;
	private: System::Windows::Forms::Button^  mGoToBtn;
	private: System::Windows::Forms::Button^  mCancelBtn;


	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->mSearchTxt = (gcnew System::Windows::Forms::TextBox());
			this->mTrackList = (gcnew System::Windows::Forms::ListBox());
			this->mPlayBtn = (gcnew System::Windows::Forms::Button());
			this->mGoToBtn = (gcnew System::Windows::Forms::Button());
			this->mCancelBtn = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(12, 18);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(55, 13);
			this->label1->TabIndex = 0;
			this->label1->Text = L"Title/Artist";
			// 
			// mSearchTxt
			// 
			this->mSearchTxt->Location = System::Drawing::Point(73, 15);
			this->mSearchTxt->Name = L"mSearchTxt";
			this->mSearchTxt->Size = System::Drawing::Size(303, 20);
			this->mSearchTxt->TabIndex = 1;
			this->mSearchTxt->TextChanged += gcnew System::EventHandler(this, &SearchForm::mSearchTxt_TextChanged);
			this->mSearchTxt->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &SearchForm::mSearchTxt_KeyDown);
			// 
			// mTrackList
			// 
			this->mTrackList->FormattingEnabled = true;
			this->mTrackList->Location = System::Drawing::Point(15, 51);
			this->mTrackList->Name = L"mTrackList";
			this->mTrackList->Size = System::Drawing::Size(361, 368);
			this->mTrackList->TabIndex = 2;
			this->mTrackList->SelectedIndexChanged += gcnew System::EventHandler(this, &SearchForm::mTrackList_SelectedIndexChanged);
			// 
			// mPlayBtn
			// 
			this->mPlayBtn->Location = System::Drawing::Point(15, 436);
			this->mPlayBtn->Name = L"mPlayBtn";
			this->mPlayBtn->Size = System::Drawing::Size(75, 23);
			this->mPlayBtn->TabIndex = 3;
			this->mPlayBtn->Text = L"P&lay";
			this->mPlayBtn->UseVisualStyleBackColor = true;
			this->mPlayBtn->Click += gcnew System::EventHandler(this, &SearchForm::mPlayBtn_Click);
			// 
			// mGoToBtn
			// 
			this->mGoToBtn->Location = System::Drawing::Point(96, 436);
			this->mGoToBtn->Name = L"mGoToBtn";
			this->mGoToBtn->Size = System::Drawing::Size(75, 23);
			this->mGoToBtn->TabIndex = 4;
			this->mGoToBtn->Text = L"&Go To";
			this->mGoToBtn->UseVisualStyleBackColor = true;
			this->mGoToBtn->Click += gcnew System::EventHandler(this, &SearchForm::mGoToBtn_Click);
			// 
			// mCancelBtn
			// 
			this->mCancelBtn->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->mCancelBtn->Location = System::Drawing::Point(301, 436);
			this->mCancelBtn->Name = L"mCancelBtn";
			this->mCancelBtn->Size = System::Drawing::Size(75, 23);
			this->mCancelBtn->TabIndex = 5;
			this->mCancelBtn->Text = L"C&ancel";
			this->mCancelBtn->UseVisualStyleBackColor = true;
			this->mCancelBtn->Click += gcnew System::EventHandler(this, &SearchForm::mCancelBtn_Click);
			// 
			// SearchForm
			// 
			this->AcceptButton = this->mPlayBtn;
			this->CancelButton = this->mCancelBtn;
			this->ClientSize = System::Drawing::Size(391, 468);
			this->Controls->Add(this->mCancelBtn);
			this->Controls->Add(this->mGoToBtn);
			this->Controls->Add(this->mPlayBtn);
			this->Controls->Add(this->mTrackList);
			this->Controls->Add(this->mSearchTxt);
			this->Controls->Add(this->label1);
			this->Name = L"SearchForm";
			this->Text = L"Search";
			this->Shown += gcnew System::EventHandler(this, &SearchForm::SearchForm_Shown);
			this->FormClosed += gcnew System::Windows::Forms::FormClosedEventHandler(this, &SearchForm::SearchForm_FormClosed);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: 
		System::Void mSearchTxt_TextChanged(System::Object^  sender, System::EventArgs^  e)
		{
			mTrackList->Items->Clear();
			mPlayBtn->Enabled = false;
			mGoToBtn->Enabled = false;

			if (mSearchTxt->Text->Length > 0)
			{
				marshal_context ^ context = gcnew marshal_context();
				const char* str4 = context->marshal_as<const char*>(mSearchTxt->Text);
				std::string searchStr = str4;
				std::vector<Track*> tracks = DatabaseManager::instance()->searchTracks(searchStr);
				
				for (unsigned int i = 0; i < tracks.size(); i++)
				{
					ListBoxItem^ item = gcnew ListBoxItem();
					item->mId = tracks[i]->Id;
					item->mString = gcnew System::String((tracks[i]->Artist + " - " + tracks[i]->Title).c_str());
	
					delete tracks[i];
					mTrackList->Items->Add(item);
				}
				if (tracks.size() > 0)
				{
					mTrackList->SetSelected(0, true);
				}
				delete context;
			}
		}
private: 
	System::Void mPlayBtn_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		mDoPlay = false;
		mDoGoTo = false;
		int sel = mTrackList->SelectedIndex;
		if (sel >= 0)
		{
			mSelectedTrackID = ((ListBoxItem^)mTrackList->SelectedItem)->mId;
			mDoPlay = true;
		}
		this->Close();
	}
private:
	System::Void mGoToBtn_Click(System::Object^  sender, System::EventArgs^  e)
	{
		int sel = mTrackList->SelectedIndex;
		mDoPlay = false;
		mDoGoTo = false;
		if (sel >= 0)
		{
			mSelectedTrackID = ((ListBoxItem^)mTrackList->SelectedItem)->mId;
			mDoGoTo = true;
		}
		this->Close();
	}

private: 
	System::Void mCancelBtn_Click(System::Object^  sender, System::EventArgs^  e)
	{
		mDoPlay = false;
		mDoGoTo = false;

		this->Close();
	}
private: 
	System::Void mTrackList_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) 
	{
		int sel = mTrackList->SelectedIndex;
		if (sel >= 0)
		{
			mPlayBtn->Enabled = true;
			mGoToBtn->Enabled = true;
		}
		else
		{
			mPlayBtn->Enabled = false;
			mGoToBtn->Enabled = false;
		}
	 }
private: 
	System::Void SearchForm_FormClosed(System::Object^  sender, System::Windows::Forms::FormClosedEventArgs^  e)
	{
		mIsOpen = false;
	}
private: 
	System::Void SearchForm_Shown(System::Object^  sender, System::EventArgs^  e) 
	 {
		 mIsOpen = true;
	}
private: 
	System::Void mSearchTxt_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e)
	{
		if (e->KeyCode == System::Windows::Forms::Keys::Down && mTrackList->Items->Count > 0)
		{
			mTrackList->Focus();
		}
	}
};
}
