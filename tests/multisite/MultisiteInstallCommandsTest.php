<?php
/**
 * @file
 * PHPUnit tests for Bee multisite Install commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's installation command in multisite.
 */
class MultisiteInstallCommandsTest extends TestCase {

  /**
   * Make sure that the install command works for multisites.
   */
  public function test_install_command_works() {
    global $bee_test_db_host, $bee_test_multisite_install_test_db_name;
    // Check site status before install.
    $output_before = shell_exec('bee status --site=install_test');
    $this->assertRegExp('/Site type +Multisite/', (string) $output_before);
    $this->assertRegExp('/Site directory +install_test/', (string) $output_before);
    $this->assertStringNotContainsString('Database', (string) $output_before);

    // Install the site.
    $output_install = shell_exec("bee install --site=install_test --db-name=$bee_test_multisite_install_test_db_name --db-user=backdrop --db-pass=backdrop --db-host=$bee_test_db_host --auto");
    $this->assertStringContainsString('Backdrop installed successfully.', (string) $output_install);

    // Check site status after install.
    $output_after = shell_exec('bee status --site=install_test');
    $this->assertRegExp('/Site type +Multisite/', (string) $output_after);
    $this->assertRegExp('/Site directory +install_test/', (string) $output_after);
    $this->assertRegExp('/Database name +install_test/', (string) $output_after);
    $this->assertRegExp('/Database host +database/', (string) $output_after);

    // Cleanup the install.
    exec('rm -r sites/install_test/files');
    exec('cp settings.php sites/install_test');
    exec("mysql -h $bee_test_db_host -u root -e 'DROP DATABASE $bee_test_multisite_install_test_db_name; CREATE DATABASE $bee_test_multisite_install_test_db_name; GRANT ALL PRIVILEGES ON $bee_test_multisite_install_test_db_name.* TO \"backdrop\"@\"%\" IDENTIFIED by \"backdrop\";'");
  }

}
