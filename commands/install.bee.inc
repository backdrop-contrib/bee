<?php
/**
 * @file
 * Command(s) for installing Backdrop.
 */

/**
 * Implements hook_bee_command().
 */
function install_bee_command() {
  return array(
    'install' => array(
      'description' => bt('Install Backdrop and setup a new site.'),
      'callback' => 'install_bee_callback',
      'options' => array(
        'db' => array(
          'description' => bt("A database connection string of the form: 'mysql://user:pass@localhost/database_name'. See https://api.backdropcms.org/database-configuration for more details."),
          'value' => bt('String'),
        ),
        'username' => array(
          'description' => bt("The username for the primary administrator account. Defaults to 'admin'."),
          'value' => bt('Username'),
        ),
        'password' => array(
          'description' => bt('The password for the primary administrator account. Defaults to a randomly-generated string.'),
          'value' => bt('Password'),
        ),
        'email' => array(
          'description' => bt("The email address for the primary administrator account. Defaults to 'admin@example.com'."),
          'value' => bt('Email'),
        ),
        'site-name' => array(
          'description' => bt("The name of the site. Defaults to 'My Backdrop Site'."),
          'value' => bt('Name'),
        ),
        'site-mail' => array(
          'description' => bt("The 'From' address used in automated site emails. Defaults to 'admin@example.com'."),
          'value' => bt('Email'),
        ),
        'profile' => array(
          'description' => bt("The machine-name of the installation profile to use. Defaults to 'standard'."),
          'value' => bt('Profile'),
        ),
        'langcode' => array(
          'description' => bt("The short code of the default site language. Language files must already be present. Defaults to 'en'."),
          'value' => bt('Language'),
        ),
        'db-prefix' => array(
          'description' => bt('The table prefix to use for this site in the database. Defaults to no prefix.'),
          'value' => bt('Prefix'),
        ),
        'no-clean-urls' => array(
          'description' => bt('Disable clean URLs.'),
        ),
        'auto' => array(
          'description' => bt('Perform an automatic (i.e. non-interactive) installation. Any options not explicitly provided to the command will use default values, except the database connection string which will always prompt when not provided.'),
        ),
      ),
      'aliases' => array('si', 'site-install'),
      'examples' => array(
        'bee install' => bt('Install Backdrop in interactive mode, providing information when prompted.'),
        'bee install --db=mysql://admin:P@ssw0rd!@localhost/backdrop --auto' => bt('Install Backdrop automatically using the provided database credentials, and default settings for everything else.'),
        'bee install --db=mysql://admin:P@ssw0rd!@localhost/backdrop --username=Root --password=N0tS3cur3 --email=root@mydomain.com --site-name="My awesome site!"' => bt('Install Backdrop using the given options, and be prompted for the rest.'),
      ),
    ),
  );
}

/**
 * Command callback: Install Backdrop and setup a new site.
 */
function install_bee_callback($arguments, $options) {
  global $_bee_backdrop_root, $_bee_backdrop_site, $_bee_multisites;
  $prompt = empty($options['auto']);

  // Make sure $_bee_backdrop_root is set.
  if (empty($_bee_backdrop_root)) {
    bee_message(bt("The install script could not be found. Re-run the command from within a Backdrop installation, or set the global '--root' option."), 'error');
    return;
  }

  // Display message about interactive mode.
  if ($prompt) {
    bee_render_text(array('value' => bt("Running installation in interactive mode. To run automatically (i.e. non-interactively), use the '--auto' option.\n")));
  }

  // Get database connection string.
  if (!empty($options['db'])) {
    // Split the passed URL into parts so can encode user, password and db name
    $db_url = $options['db'];
    $db_name = rawurlencode(bee_check_plain(bee_after_last('/', $db_url)));
    $db_username = rawurlencode(bee_check_plain(bee_before(':', bee_after('mysql://', $db_url))));
    $db_password = rawurlencode(bee_check_plain(bee_between($db_username . ':', '@' . bee_after_last('@', $db_url), $db_url)));
    $db_host = bee_check_plain(bee_between_last('@', '/', $db_url));
    // Re-compile the database URL using the sanitized parts
    $db = "mysql://$db_username:$db_password@$db_host/$db_name";
  }
  else {
    $db_name = rawurlencode(bee_check_plain(bee_input(bt('Database name'), '', TRUE)));
    $db_username = rawurlencode(bee_check_plain(bee_input(bt('Database username'), '', TRUE)));
    $db_password = rawurlencode(bee_check_plain(bee_input(bt('Database password'), '', TRUE)));
    $db_host = bee_check_plain(bee_input(bt('Database host'), 'localhost', TRUE));
    $db = "mysql://$db_username:$db_password@$db_host/$db_name";
  }
  $db = "--db-url=$db";

  // Get database prefix.
  if (!empty($options['db-prefix'])) {
    $db_prefix = $options['db-prefix'];
  }
  else {
    $db_prefix = ($prompt) ? bee_check_plain(bee_input(bt('Database prefix'), '')) : '';
  }
  $db_prefix = "--db-prefix=$db_prefix";

  // Get admin username.
  if (!empty($options['username'])) {
    $username = $options['username'];
  }
  else {
    $username = ($prompt) ? bee_check_plain(bee_input(bt('Admin username'), 'admin', TRUE)) : 'admin';
  }
  $username = "--account-name='$username'";

  // Get admin password.
  $random = substr(md5(rand()), 0, 20);
  if (!empty($options['password'])) {
    $password = $options['password'];
  }
  else {
    $password = ($prompt) ? bee_check_plain(bee_input(bt('Admin password'), $random, TRUE)) : $random;
  }
  $show_password = ($password == $random) ? TRUE : FALSE;
  $password = "--account-pass=$password";

  // Get admin email.
  if (!empty($options['email'])) {
    $email = $options['email'];
  }
  else {
    $email = ($prompt) ? bee_check_plain(bee_input(bt('Admin email'), 'admin@example.com', TRUE)) : 'admin@example.com';
  }
  $email = "--account-mail=$email";

  // Get site name.
  if (!empty($options['site-name'])) {
    $site_name = $options['site-name'];
  }
  else {
    $site_name = ($prompt) ? bee_check_plain(bee_input(bt('Site name'), 'My Backdrop Site', TRUE)) : 'My Backdrop Site';
  }
  $site_name = "--site-name='$site_name'";

  // Get site mail.
  if (!empty($options['site-mail'])) {
    $site_mail = $options['site-mail'];
  }
  else {
    $site_mail = ($prompt) ? bee_check_plain(bee_input(bt('Site mail'), 'admin@example.com', TRUE)) : 'admin@example.com';
  }
  $site_mail = "--site-mail=$site_mail";

  // Get language.
  if (!empty($options['langcode'])) {
    $langcode = $options['langcode'];
  }
  else {
    $langcode = ($prompt) ? bee_check_plain(bee_input(bt('Language code'), 'en', TRUE)) : 'en';
  }
  $langcode = "--langcode=$langcode";

  // Get clean URLs.
  $clean_urls = (!empty($options['no-clean-urls'])) ? 0 : 1;
  $clean_urls = "--clean-url=$clean_urls";

  // Get profile.
  if (!empty($options['profile'])) {
    $profile = $options['profile'];
  }
  else {
    $profile = ($prompt) ? bee_check_plain(bee_input(bt('Install profile'), 'standard', TRUE)) : 'standard';
  }

  // Get multisite site URL.
  $url = '';
  if ($_bee_backdrop_site) {
    $url = '--url=' . array_search($_bee_backdrop_site, $_bee_multisites);
  }

  // Run the install script.
  bee_render_text(array('value' => bt("\nInstalling Backdrop. This may take a moment...\n")));
  $result = exec("cd $_bee_backdrop_root && ./core/scripts/install.sh $url $db $db_prefix $username $password $email $site_name $site_mail $langcode $clean_urls $profile");
  // Strip tags to prevent HTML code being output.
  bee_render_text(array('value' => strip_tags($result)));

  // Show the randomly-generated password.
  if (strpos($result, 'successfully') !== FALSE && $show_password) {
    bee_render_text(array('value' => bt('The randomly-generated admin password is: ' . $random)), FALSE);
  }
}

/**
 * Encodes special characters in a plain-text string.
 *
 * @see check_plain()
 *
 * Copied here so it can be used while the Backdrop site is being installed.
 */
function bee_check_plain($text) {
  return htmlspecialchars($text, ENT_QUOTES, 'UTF-8');
}

/**
 * Helper functions for extracting the database connection parts from db option
 *
 * Source: https://www.php.net/manual/en/function.substr.php#112707
 */

// Returns all in $string between $text1 and $text2
function bee_between ($text1, $text2, $string)
{
    return bee_before ($text2, bee_after($text1, $string));
};

// Returns all in $string between the last @text1 and last $text2
function bee_between_last ($text1, $text2, $string)
{
 return bee_after_last($text1, bee_before_last($text2, $string));
};

// Returns all in $string after $text
function bee_after ($text, $string)
{
    if (!is_bool(strpos($string, $text)))
    return substr($string, strpos($string,$text)+strlen($text));
};

// Returns all in $string after the last occurrence of $text
function bee_after_last ($text, $string)
{
    if (!is_bool(bee_position_from_end($string, $text)))
    return substr($string, bee_position_from_end($string, $text)+strlen($text));
};

// Returns all in $string before $text
function bee_before ($text, $string)
{
    return substr($string, 0, strpos($string, $text));
};

// Returns all in $string before the last occurrence of $text
function bee_before_last ($text, $string)
{
    return substr($string, 0, bee_position_from_end($string, $text));
};

// Returns position of $text from the end of $string or `false` if not found
function bee_position_from_end($string, $text)
{
$rev_pos = strpos (strrev($string), strrev($text));
if ($rev_pos===false) return false;
else return strlen($string) - $rev_pos - strlen($text);
};
