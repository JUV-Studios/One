// (c) 2021 JUV Studios. All rights reserved. Included as part of OneToolkit for use in C++ projects targeting the Windows platform.

#pragma once
#include <juv.h>
#include <concepts>
#include <winerror.h>
#include <debugapi.h>
#include <winrt/OneToolkit.UI.h>
#include <winrt/OneToolkit.Mvvm.h>
#include <winrt/OneToolkit.Imaging.h>
#include <winrt/OneToolkit.Storage.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/OneToolkit.Lifecycle.h>
#include <winrt/OneToolkit.Data.Text.h>

#define DeclareEvent(Type, Name) private: ::winrt::event<Type> m_##Name;\
public: ::winrt::event_token Name(Type const& handler) { return m_##Name.add(handler); }\
void Name(::winrt::event_token token) noexcept { m_##Name.remove(token); }

#define DeclareObservableProperty(Type, Name, DefaultValue) private: Type m_##Name = DefaultValue;\
public: Type Name() const noexcept { return m_##Name; }\
void Name(Type value) { SetProperty<Type>(m_##Name, value, L#Name); }

namespace winrt::OneToolkit
{
	/// <summary>
	/// Represents a type which can be passed through the Windows Runtime ABI.
	/// </summary>
	template <typename T>
	concept WindowsRuntimeType = winrt::impl::has_category_v<T>;

	namespace Debugger
	{
		inline void Break() noexcept
		{
			DebugBreak();
		}

		inline bool IsAttached() noexcept
		{
			return IsDebuggerPresent() != 0;
		}

		inline void Write(std::string_view text) noexcept
		{
			OutputDebugStringA(text.data());
		}

		inline void Write(std::wstring_view text) noexcept
		{
			OutputDebugStringW(text.data());
		}

		inline void WriteLine(std::string& line)
		{
			OutputDebugStringA((line + "\n").data());
		}

		inline void WriteLine(std::wstring& line)
		{
			OutputDebugStringW((line + L"\n").data());
		}
	}

	namespace Runtime
	{
		/// <summary>
		/// Represents a dynamic link library and enables the ability to use exported functions or variables.
		/// </summary>
		class DynamicModule
		{
		public:
			DynamicModule(std::string_view fileName) : DynamicModule(to_hstring(fileName))
			{
			}
	
			DynamicModule(winrt::hstring const& fileName) : m_FileName(fileName)
			{
				Handle(WINRT_IMPL_LoadLibraryW(fileName.data()));
			}

			DynamicModule(DynamicModule const& another)
			{
				Copy(another);
			}

			DynamicModule(DynamicModule&& another) noexcept
			{
				Move(std::move(another));
			}

			DynamicModule& operator=(const DynamicModule& another)
			{
				Copy(another);
				return *this;
			}

			DynamicModule& operator=(DynamicModule&& another) noexcept
			{
				Move(std::move(another));
				return *this;
			}

			bool operator==(DynamicModule const& another) const noexcept
			{
				return m_FileName == another.m_FileName && m_Handle == another.m_Handle;
			}

			bool operator!=(DynamicModule const& another) const noexcept
			{
				return !operator==(another);
			}

			/// <summary>
			/// Returns an handle to the loaded dynamic link library.
			/// </summary>
			auto Handle() const noexcept
			{
				return m_Handle;
			}

			/// <summary>
			/// Returns the name of the DLL.
			/// </summary>
			const hstring FileName() const noexcept
			{
				return m_FileName;
			}

			/// <summary>
			/// Retrieves the address of an exported function or variable.
			/// </summary>
			template <typename T>
			auto GetProcAddress(std::string_view procName) const
			{
				auto result = WINRT_IMPL_GetProcAddress(m_Handle, procName.data());
				if (!result) throw_last_error();
				return reinterpret_cast<T>(result);
			}

			/// <summary>
			/// Retrieves the address of an exported function or variable.
			/// </summary>
			template <typename T>
			auto GetProcAddress(std::wstring_view procName) const
			{
				auto procNameA = to_string(procName);
				return GetProcAddress<T>(procNameA);
			}
	
			~DynamicModule()
			{
				if (m_Handle) WINRT_IMPL_FreeLibrary(m_Handle);
			}
		private:
			HMODULE m_Handle;

			hstring m_FileName;

			void Handle(void* newHandle)
			{
				if (!newHandle) throw_last_error();
				m_Handle = static_cast<HMODULE>(newHandle);
			}

			void Copy(DynamicModule const& another)
			{
				Handle(WINRT_IMPL_LoadLibraryW(another.m_FileName.data()));
				m_FileName = another.m_FileName;
			}

			void Move(DynamicModule&& another) noexcept
			{
				Handle(another.m_Handle);
				m_FileName = another.m_FileName;
				another.m_Handle = nullptr;
			}
		};
	}

	namespace Mvvm
	{
		enum class PropertyEventType : juv::uint8
		{
			PropertyChanging, PropertyChanged
		};

		/// <summary>
		/// Provides a base class for view models and observable objects.
		/// </summary>
		/// <remarks> If you're implementing a runtime class, we recommend that you implement INotifyPropertyChanged, INotifyPropertyChanging and/or IEventDisableable.</remarks>
		template <typename Derived, WindowsRuntimeType ChangedDelegate = Windows::UI::Xaml::Data::PropertyChangedEventHandler, WindowsRuntimeType ChangedArgs = Windows::UI::Xaml::Data::PropertyChangedEventArgs>
		struct ObservableBase
		{
		public:
			ObservableBase(ObservableBase&&) = delete;

			ObservableBase(ObservableBase const&) = delete;

			/// <summary>
			/// Gets whether the property changing/property changed events will be raised or not.
			/// </summary>
			bool SuppressEvents() const noexcept
			{
				return m_SuppressEvents;
			}

			DeclareEvent(ChangedDelegate, PropertyChanged);

			DeclareEvent(PropertyChangingEventHandler, PropertyChanging);
		protected:
			ObservableBase(bool suppressEvents = false) : m_SuppressEvents(suppressEvents)
			{
			}

			void SuppressEvents(bool value) noexcept
			{
				m_SuppressEvents = value;
			}

			/// <summary>
			/// Raises the specified property event for a specified property name.
			/// </summary>
			/// <param name="propertyName">The name of the property. Shouldn't be empty or only full of whitespaces.</param>
			/// <param name="type">The type of the property event.</param>
			void Raise(hstring const& propertyName, PropertyEventType eventType = PropertyEventType::PropertyChanged)
			{
				if (!juv::has_only_whitespaces(propertyName) && Decide(propertyName, eventType) && !m_SuppressEvents)
				{
					if (eventType == PropertyEventType::PropertyChanging)
					{
						PropertyChangingEventArgs args{ .PropertyName = propertyName };
						m_PropertyChanging(*static_cast<Derived*>(this), args);
						WhenPropertyChanging(args);
					}
					else
					{
						ChangedArgs args{ propertyName };
						m_PropertyChanged(*static_cast<Derived*>(this), args);
						WhenPropertyChanged(args);
					}
				}
			}

			/// <summary>
			/// Automatically sets a property value and raises property changing/property changed when required.
			/// </summary>
			/// <returns>True if the value was set, false if the value passed was equal to the fields existing value.</returns>
			template <typename T>
			bool SetProperty(T& field, T newValue, hstring const& propertyName)
			{
				if (field != newValue)
				{
					Raise(propertyName, PropertyEventType::PropertyChanging);
					field = newValue;
					Raise(propertyName, PropertyEventType::PropertyChanged);
					return true;
				}

				return false;
			}

			/// <summary>
			/// Override this method to determine whether to raise property changed/property changing or not.
			/// </summary>
			virtual bool Decide(hstring const&, PropertyEventType) noexcept
			{
				return true;
			}

			/// <summary>
			/// Override this method to perform custom actions after raising the property changed event.
			/// </summary>
			virtual void WhenPropertyChanged(ChangedArgs const&)
			{
				// Do nothing here.
			}

			/// <summary>
			/// Override this method to perform custom actions after raising the property changing event.
			/// </summary>
			virtual void WhenPropertyChanging(PropertyChangingEventArgs const&)
			{
				// Do nothing here.
			}
		private:
			bool m_SuppressEvents;
		};

		template <typename T>
		class AutoProperty
		{
		public:
			AutoProperty(T defaultValue = {}) : m_BackingField(defaultValue)
			{
			}

			T operator()() const noexcept
			{
				return m_BackingField;
			}

			void operator()(T value)
			{
				m_BackingField = value;
			}

			T& operator*() const noexcept
			{
				return m_BackingField;
			}

			T* operator->() const noexcept
			{
				return &m_BackingField;
			}
		private:
			T m_BackingField;
		};
	}

	namespace Lifecycle
	{
		/// <summary>
		/// Provides a base class to conveniently implement the IClosable interface.
		/// </summary>
		/// <remarks>Your derived class must provide a Dispose method which will be called by Close when the object hasn't been closed yet.</remarks>
		template <typename Derived>
		struct Disposable
		{
		public:
			Disposable(Disposable&&) = delete;

			Disposable(Disposable const&) = delete;

			void Close()
			{
				if (!m_IsDisposed)
				{
					static_cast<Derived*>(this)->Dispose();
					m_IsDisposed = true;
				}
			}

			~Disposable()
			{
				Close();
			}
		protected:
			Disposable()
			{
			}

			/// <summary>
			/// Throws an exception if the object is closed.
			/// </summary>
			inline void ThrowIfDisposed() const
			{
				if (m_IsDisposed) throw hresult_error(RO_E_CLOSED);
			}
		private:
			bool m_IsDisposed;
		};

		/// <summary>
		/// Provides a base class to conveniently implement the ISuspendable interface.
		/// </summary>
		template <typename Derived>
		struct Suspendable
		{
		public:
			Suspendable(Suspendable&&) = delete;

			Suspendable(Suspendable const&) = delete;

			/// <summary>
			/// Gets whether the current object is suspended.
			/// </summary>
			bool IsSuspended() const noexcept
			{
				return m_IsSuspended;
			}

			DeclareEvent(SuspendableStateChangedEventHandler, StateChanged);
		protected:
			Suspendable() = default;

			/// <summary>
			/// Toggles the suspended state.
			/// </summary>
			void ToggleState()
			{
				if (m_IsSuspended)
				{
					m_IsSuspended = false;
					m_StateChanged(*static_cast<Derived*>(this), SuspendableEventType::Resumed);
				}
				else
				{
					m_IsSuspended = true;
					m_StateChanged(*static_cast<Derived*>(this), SuspendableEventType::Suspended);
				}
			}
		private:
			bool m_IsSuspended;
		};
		
		/// <summary>
		/// Provides a base class to conveniently implement the IAsyncInitialize interface. 
		/// </summary>
		template <typename Derived>
		struct AsyncInitialize : Mvvm::ObservableBase<Derived>
		{
		public:
			AsyncInitialize(AsyncInitialize&&) = delete;

			AsyncInitialize(AsyncInitialize const&) = delete;

			DeclareObservableProperty(bool, IsLoading, false);

			DeclareObservableProperty(bool, HasInitialized, false);
		protected:
			AsyncInitialize() = default;
		};

		/// <summary>
		/// Provides a base class to conveniently implement the IEquatable interface.
		/// </summary>
		/// <remarks>Your derived class must provide an overload of the equals operator that takes an IInspectable.</remarks>
		template <typename Derived>
		struct Equatable
		{
		public:
			Equatable(Equatable&&) = delete;

			Equatable(Equatable const&) = delete;

			bool IsEqual(Windows::Foundation::IInspectable const& another) const noexcept
			{
				if (another == *static_cast<const Derived*>(this)) return true;
				else return static_cast<const Derived*>(this)->operator==(another);
			}
		protected:
			Equatable() = default;
		};
	}
}