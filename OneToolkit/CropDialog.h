﻿#pragma once
#include "Imaging.CropDialog.g.h"

namespace winrt::OneToolkit::Imaging
{
    namespace implementation
    {
        struct CropDialog : CropDialogT<CropDialog>
        {
            CropDialog() = default;
            static Windows::Foundation::IAsyncOperation<bool> IsSupportedAsync();
            Windows::Foundation::IAsyncOperation<bool> CropAsync(Windows::Storage::StorageFile input, Windows::Storage::StorageFile destination) const;
            Mvvm::AutoProperty<bool> ShowCamera;
            Mvvm::AutoProperty<bool> IsEllipticalCrop;
            Mvvm::AutoProperty<Windows::Foundation::Size> CropSize { {500, 500} };
        };
    }

    namespace factory_implementation
    {
        struct CropDialog : CropDialogT<CropDialog, implementation::CropDialog>
        {
        };
    }
}