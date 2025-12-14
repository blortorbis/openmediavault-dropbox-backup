<?php
/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Gabe Burke <gabeburke@gmail.com>
 * @copyright Copyright (c) 2024 Gabrael Burke
 *
 * OpenMediaVault Dropbox Backup plugin - RPC service
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 */

namespace Engined\Rpc;

use OMV\Config\Database;
use OMV\Engine\Notify\Dispatcher;
use OMV\Rpc\ServiceAbstract;
use OMV\System\Process;

class DropboxBackup extends ServiceAbstract {
    public function getName() {
        return "DropboxBackup";
    }

    public function initialize() {
        $this->registerMethod("get");
        $this->registerMethod("set");
        $this->registerMethod("getAuthStatus");
        $this->registerMethod("startAuth");
        $this->registerMethod("unlinkAccount");
        $this->registerMethod("runBackup");
        $this->registerMethod("getBackupStatus");
    }

    /**
     * Get the configuration settings.
     */
    public function get($params, $context) {
        return \OMV\Rpc\Rpc::call("Config", "get", [
            "id" => "conf.service.dropbox-backup"
        ], $context);
    }

    /**
     * Set the configuration settings.
     */
    public function set($params, $context) {
        return \OMV\Rpc\Rpc::call("Config", "set", [
            "id" => "conf.service.dropbox-backup",
            "data" => $params
        ], $context);
    }

    /**
     * Check if Dropbox is configured in rclone.
     */
    public function getAuthStatus($params, $context) {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        // Check if rclone has a dropbox remote configured
        $cmd = "rclone listremotes 2>/dev/null | grep -q '^dropbox:$' && echo 'linked' || echo 'unlinked'";
        $process = new Process($cmd);
        $process->execute($output);
        $status = trim($output[0] ?? 'unlinked');

        $accountInfo = null;
        if ($status === 'linked') {
            // Get account info
            $cmd = "rclone about dropbox: --json 2>/dev/null";
            $process = new Process($cmd);
            try {
                $process->execute($output);
                $accountInfo = json_decode(implode("", $output), true);
            } catch (\Exception $e) {
                // Ignore errors
            }
        }

        return [
            "linked" => ($status === 'linked'),
            "accountInfo" => $accountInfo
        ];
    }

    /**
     * Start the rclone OAuth authorization flow.
     * Returns a URL that the user should visit.
     */
    public function startAuth($params, $context) {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        // Generate rclone config command
        // Note: This creates a headless auth flow - user will need to
        // copy the auth URL and paste the token back
        $cmd = "rclone authorize dropbox 2>&1";

        return [
            "message" => "Run the following command on a machine with a browser to authenticate:\n\nrclone authorize dropbox\n\nThen configure the remote manually with:\n\nrclone config create dropbox dropbox",
            "command" => $cmd
        ];
    }

    /**
     * Remove the Dropbox remote from rclone config.
     */
    public function unlinkAccount($params, $context) {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        $cmd = "rclone config delete dropbox 2>&1";
        $process = new Process($cmd);
        $process->execute($output);

        return [
            "success" => true,
            "output" => implode("\n", $output)
        ];
    }

    /**
     * Run a backup now.
     */
    public function runBackup($params, $context) {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        // Get current config
        $config = $this->get([], $context);

        if (!$config['enable']) {
            throw new \OMV\Exception("Dropbox backup is not enabled");
        }

        // Run the backup script in background
        $cmd = "/usr/sbin/omv-dropbox-backup &";
        $process = new Process($cmd);
        $process->execute();

        return [
            "success" => true,
            "message" => "Backup started in background"
        ];
    }

    /**
     * Get backup status (last run, etc).
     */
    public function getBackupStatus($params, $context) {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        $statusFile = "/var/lib/openmediavault/dropbox-backup-status.json";
        $status = [
            "lastRun" => null,
            "lastStatus" => "unknown",
            "lastMessage" => null
        ];

        if (file_exists($statusFile)) {
            $content = file_get_contents($statusFile);
            $status = array_merge($status, json_decode($content, true) ?? []);
        }

        // Check if backup is currently running
        $cmd = "pgrep -f 'omv-dropbox-backup' > /dev/null && echo 'running' || echo 'idle'";
        $process = new Process($cmd);
        $process->execute($output);
        $status["running"] = (trim($output[0] ?? '') === 'running');

        return $status;
    }
}
