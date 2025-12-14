# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Gabrael Burke <gabrael@example.com>
# @copyright Copyright (c) 2024 Gabrael Burke
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.

{% set config = salt['omv_conf.get']('conf.service.dropbox-backup') %}

configure_dropbox_backup_timer:
  service.{{ 'enabled' if config.enable and config.schedule != 'disabled' else 'disabled' }}:
    - name: omv-dropbox-backup.timer
    - enable: {{ config.enable and config.schedule != 'disabled' }}
