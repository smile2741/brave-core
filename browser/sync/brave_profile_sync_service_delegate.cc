/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/sync/brave_profile_sync_service_delegate.h"

#include <utility>

#include "base/task/post_task.h"
#include "brave/components/sync/driver/brave_sync_profile_sync_service.h"
#include "chrome/browser/sync/device_info_sync_service_factory.h"
#include "chrome/browser/sync/profile_sync_service_factory.h"
#include "components/sync_device_info/device_info_sync_service.h"
#include "components/sync_device_info/device_info_tracker.h"
#include "components/sync_device_info/local_device_info_provider.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"

namespace syncer {

namespace {
syncer::BraveProfileSyncService* GetSyncService(Profile* profile) {
  return ProfileSyncServiceFactory::IsSyncAllowed(profile)
             ? static_cast<syncer::BraveProfileSyncService*>(
                   ProfileSyncServiceFactory::GetForProfile(profile))
             : nullptr;
}
}  // namespace

BraveProfileSyncServiceDelegate::BraveProfileSyncServiceDelegate(
    Profile* profile)
    : device_info_sync_service_(
          DeviceInfoSyncServiceFactory::GetForProfile(profile)),
      profile_(profile),
      weak_ptr_factory_(this) {
  DCHECK(device_info_sync_service_);

  local_device_info_provider_ =
      device_info_sync_service_->GetLocalDeviceInfoProvider();

  device_info_tracker_ = device_info_sync_service_->GetDeviceInfoTracker();
  DCHECK(device_info_tracker_);

  device_info_observer_.Add(device_info_tracker_);
}

BraveProfileSyncServiceDelegate::~BraveProfileSyncServiceDelegate() {}

void BraveProfileSyncServiceDelegate::OnDeviceInfoChange() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  const syncer::DeviceInfo* local_device_info =
      local_device_info_provider_->GetLocalDeviceInfo();

  bool found_local_device = false;
  const auto all_devices = device_info_tracker_->GetAllDeviceInfo();
  for (const auto& device : all_devices) {
    if (local_device_info->guid() == device->guid()) {
      found_local_device = true;
      break;
    }
  }

  // When our device was removed from the sync chain by some other device,
  // we don't seee it in devices list, we must reset sync in a proper way
  if (!found_local_device) {
    // We can't call OnSelfDeviceInfoDeleted directly because we are on
    // remove device execution path, so posting task
    base::PostTask(
        FROM_HERE, {content::BrowserThread::UI},
        base::BindOnce(
            &BraveProfileSyncServiceDelegate::OnSelfDeviceInfoDeleted,
            weak_ptr_factory_.GetWeakPtr()));
  }
}

void BraveProfileSyncServiceDelegate::OnSelfDeviceInfoDeleted(void) {
  syncer::BraveProfileSyncService* sync_service = GetSyncService(profile_);
  DCHECK(sync_service);
//  sync_service->OnSelfDeviceInfoDeleted(base::DoNothing::Once());
}

void BraveProfileSyncServiceDelegate::SuspendDeviceObserverForOwnReset() {
  if (device_info_observer_.IsObserving(device_info_tracker_)) {
    device_info_observer_.Remove(device_info_tracker_);
  }
}

void BraveProfileSyncServiceDelegate::ResumeDeviceObserver() {
  if (!device_info_observer_.IsObserving(device_info_tracker_)) {
    device_info_observer_.Add(device_info_tracker_);
  }
}

}  // namespace syncer
