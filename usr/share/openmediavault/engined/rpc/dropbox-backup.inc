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

use OMV\Rpc\ServiceAbstract;

class DropboxBackup extends ServiceAbstract {
    public function getName() {
        return "DropboxBackup";
    }

    public function initialize() {
        $this->registerMethod("get");
        $this->registerMethod("set");
        $this->registerMethod("runBackup");
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
     * Run a backup now as a background task. The workbench task dialog
     * polls the output file to show live progress.
     */
    public function runBackup($params, $context) {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        // Make sure the service is enabled, otherwise the backup
        // script exits without doing anything.
        $config = $this->get([], $context);
        if (TRUE !== $config['enable']) {
            throw new \OMV\Exception("Dropbox backup is not enabled.");
        }

        return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename) {
            $cmd = new \OMV\System\Process("/usr/sbin/omv-dropbox-backup");
            $cmd->setRedirect2to1();
            $cmdLine = $cmd->getCommandLine();
            if (0 !== $this->exec($cmdLine, $output, $bgOutputFilename)) {
                throw new \OMV\ExecException($cmdLine, $output);
            }
            return $output;
        });
    }
}
