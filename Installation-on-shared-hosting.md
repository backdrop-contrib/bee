## Introduction

It may be possible to use Bee on your shared hosting if you have SSH access (or a virtual terminal - see Advanced > Terminal in your cPanel or equivalent) and if it will support aliases. See [this META issue](https://github.com/backdrop-contrib/bee/issues/154) for a list of shared hosts where this method has been tested and confirmed to work using the simple test above.

## Manual (based on cPanel File Manager)
- Download zip from GitHub
  - Click on green 'Code' button
  - Click on 'Download ZIP'
- Upload zip to root of your home folder on your shared hosting account
- Extract zip to root of home folder
  - Select the uploaded zip file
  - Click on 'Extract'
  - Leave the path blank
  - Click on 'Extract File(s)'
- Rename folder to 'bee'
- Add alias
  - Open .bashrc to Edit
  - Add `alias bee='~/bee/bee.php'` to the bottom of the file
  - Click 'Save Changes'

You should now be able to complete the test by accessing your backdrop folder through SSH or your virtual terminal.

## Using Git

- On your cPanel homepage, click on 'Git&#x2122; Version Control' under Files (if this isn't there in your cPanel, you won't be able to use this method and will need to use the manual method above)
- Click Create
- Ensure the switch for 'Clone a Repository' is switched on
- Copy the Clone URL from this Git repository (Code>Clone) and paste it into the 'Clone URL' field
- enter the folder name (i.e. 'bee') under 'Repository Path'
- enter a 'Repository Name; (e.g 'Bee' or 'Backdrop Bee' )
- Click 'Create'
- Add alias
  - Open .bashrc to Edit
  - Add `alias bee='~/bee/bee.php'` to the bottom of the file
  - Click 'Save Changes'

You should now be able to complete the test by accessing your backdrop folder through SSH or your virtual terminal.

### Updates
You will be able to update your installation by clicking on 'Manage' and then 'Update'.

## Troubleshooting
### Bee doesn't work after adding the alias
After adding the alias to .bashrc, Bee didn't work at all. 
#### Solution
Either:
1. Log out of your SSH session, and log back in
2. Re-load the bash configuration using the command `source .bashrc`