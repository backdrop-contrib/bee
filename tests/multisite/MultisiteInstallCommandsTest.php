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
    // Check site status before install.
    $output_before = shell_exec('bee status --site=install_test');
    $this->assertRegExp('/Site type +Multisite/', (string) $output_before);
    $this->assertRegExp('/Site directory +install_test/', (string) $output_before);
    $this->assertStringNotContainsString('Database', (string) $output_before);

    // Install the site.
    $output_install = shell_exec('bee install --site=install_test --db-name=install_test --db-user=backdrop --db-pass=backdrop --db-host=database --auto');
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
    exec('mysql -h database -u root -e "DROP DATABASE install_test; CREATE DATABASE install_test; GRANT ALL PRIVILEGES ON install_test.* TO \'backdrop\'@\'%\' IDENTIFIED by \'backdrop\';"');
  }

}
