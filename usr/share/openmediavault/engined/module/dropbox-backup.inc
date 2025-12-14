<?php
/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Gabrael Burke <gabrael@example.com>
 * @copyright Copyright (c) 2024 Gabrael Burke
 *
 * OpenMediaVault Dropbox Backup plugin - Module
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 */

namespace Engined\Module;

use OMV\Config\Database;
use OMV\Engine\Module\IServiceStatus;
use OMV\Engine\Module\Manager as ModuleManager;
use OMV\Engine\Module\ServiceAbstract;
use OMV\Engine\Notify\Dispatcher;
use OMV\Engine\Notify\IListener;
use OMV\System\Process;
use OMV\System\SystemCtl;

class DropboxBackup extends ServiceAbstract implements IServiceStatus, IListener {
    public function getName() {
        return "dropbox-backup";
    }

    public function deployAfter(): array {
        return ["systemd"];
    }

    public function getStatus() {
        $db = Database::getInstance();
        $object = $db->get("conf.service.dropbox-backup");

        // Check if timer is active
        $systemCtl = new SystemCtl("omv-dropbox-backup.timer");
        $timerEnabled = $systemCtl->isEnabled();

        return [
            "name" => $this->getName(),
            "title" => gettext("Dropbox Backup"),
            "enabled" => $object->get("enable"),
            "running" => $timerEnabled
        ];
    }

    public function startService() {
        $db = Database::getInstance();
        $object = $db->get("conf.service.dropbox-backup");

        if (TRUE !== $object->get("enable")) {
            return;
        }

        $schedule = $object->get("schedule");
        if ($schedule !== "disabled") {
            // Enable and start the systemd timer
            $systemCtl = new SystemCtl("omv-dropbox-backup.timer");
            $systemCtl->enable(TRUE);
        }
    }

    public function stopService() {
        // Disable the systemd timer
        $systemCtl = new SystemCtl("omv-dropbox-backup.timer");
        $systemCtl->disable(TRUE);
    }

    public function bindListeners(Dispatcher $dispatcher) {
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            "org.openmediavault.conf.service.dropbox-backup",
            [$this, "setDirty"]
        );

        $moduleMgr = ModuleManager::getInstance();
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            "org.openmediavault.conf.service.dropbox-backup",
            [$moduleMgr, "setModuleDirty"],
            ["dropbox-backup"]
        );
    }
}
