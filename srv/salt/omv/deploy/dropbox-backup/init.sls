# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Gabe Burke <gabeburke@gmail.com>
# @copyright Copyright (c) 2024 Gabrael Burke
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.

{% set config = salt['omv_conf.get']('conf.service.dropbox-backup') %}
{% set oncalendar = {
  'hourly': 'hourly',
  'daily': '*-*-* 02:00:00',
  'weekly': 'Sun *-*-* 02:00:00'
} %}

{% if config.enable | to_bool and config.schedule != 'disabled' %}

configure_dropbox_backup_timer_schedule:
  file.managed:
    - name: /etc/systemd/system/omv-dropbox-backup.timer.d/10-openmediavault-schedule.conf
    - makedirs: True
    - contents: |
        [Timer]
        OnCalendar=
        OnCalendar={{ oncalendar[config.schedule] }}

dropbox_backup_systemd_reload:
  module.run:
    - service.systemctl_reload:
    - onchanges:
      - file: configure_dropbox_backup_timer_schedule

start_dropbox_backup_timer:
  service.running:
    - name: omv-dropbox-backup.timer
    - enable: True
    - watch:
      - file: configure_dropbox_backup_timer_schedule

{% else %}

remove_dropbox_backup_timer_schedule:
  file.absent:
    - name: /etc/systemd/system/omv-dropbox-backup.timer.d

stop_dropbox_backup_timer:
  service.dead:
    - name: omv-dropbox-backup.timer
    - enable: False

{% endif %}
