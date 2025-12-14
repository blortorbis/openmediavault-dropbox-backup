# openmediavault-dropbox-backup

A plugin for [OpenMediaVault](https://www.openmediavault.org/) 7.x that backs up your Dropbox contents to a local folder on your NAS.

## Features

- **One-way sync**: Backs up Dropbox to local storage (Dropbox → OMV)
- **OAuth authentication**: Uses rclone's built-in Dropbox OAuth (no app registration needed)
- **Selective sync**: Back up entire Dropbox or specific folders
- **Scheduled backups**: Manual, hourly, daily, or weekly backup options
- **Web UI**: Configure everything from the OMV web interface

## Requirements

- OpenMediaVault 7.x or later
- rclone (automatically installed as dependency)

## Installation

### From .deb package

```bash
# Build the package
dpkg-buildpackage -b -us -uc

# Install
sudo dpkg -i ../openmediavault-dropbox-backup_1.0.0_all.deb
sudo apt-get install -f  # Install any missing dependencies
```

### Manual installation

1. Copy files to their destinations on your OMV server
2. Run `omv-mkworkbench all` to rebuild the web interface
3. Restart the OMV engine: `systemctl restart openmediavault-engined`

## Configuration

### Step 1: Link your Dropbox account

Before using the plugin, you need to configure rclone with your Dropbox account:

```bash
# On the OMV server, run:
rclone config

# Follow the prompts:
# - Choose 'n' for new remote
# - Name it 'dropbox' (must be exactly this name)
# - Choose 'dropbox' as the storage type
# - Leave client_id and client_secret empty (use rclone's)
# - Follow the OAuth flow in your browser
```

### Step 2: Configure the plugin

1. Go to **Services → Dropbox Backup** in the OMV web interface
2. Enable the service
3. Select a destination shared folder
4. Optionally specify a Dropbox path to sync (default: entire Dropbox)
5. Set your preferred backup schedule
6. Click Save

### Step 3: Run a backup

Click "Run Backup Now" to start an immediate backup, or wait for the scheduled backup to run.

## Advanced Options

You can pass additional rclone options in the "Extra rclone Options" field:

- `--bwlimit 1M` - Limit bandwidth to 1 MB/s
- `--exclude "*.tmp"` - Exclude temporary files
- `--dry-run` - Test without making changes

## Logs

Backup logs are written to syslog. View them with:

```bash
journalctl -t omv-dropbox-backup
```

Or check the status file:

```bash
cat /var/lib/openmediavault/dropbox-backup-status.json
```

## Uninstallation

```bash
sudo apt-get remove openmediavault-dropbox-backup
# To also remove configuration:
sudo apt-get purge openmediavault-dropbox-backup
```

## Development

### Project Structure

```text
openmediavault-dropbox-backup/
├── debian/                          # Debian packaging
├── etc/systemd/system/              # Systemd service and timer
├── usr/
│   ├── sbin/
│   │   └── omv-dropbox-backup       # Backup script
│   └── share/openmediavault/
│       ├── datamodels/              # Configuration schema
│       ├── engined/
│       │   ├── module/              # Service module
│       │   └── rpc/                 # RPC backend
│       └── workbench/               # Web UI (YAML)
└── README.md
```

### Building

```bash
dpkg-buildpackage -b -us -uc
```

## License

GPL-3.0

## Credits

- Original plugin by Lorenzo Dieryckx (2013)
- Modernized for OMV 7.x by Gabrael Burke (2024)
