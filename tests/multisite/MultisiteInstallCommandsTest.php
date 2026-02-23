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
    $this->assertMatchesRegularExpression('/Site type +Multisite/', (string) $output_before);
    $this->assertMatchesRegularExpression('/Site directory +install_test/', (string) $output_before);
    $this->assertStringNotContainsString('Database', (string) $output_before);

    // Install the site.
    $output_install = shell_exec("bee install --site=install_test --db-name=$bee_test_multisite_install_test_db_name --db-user=backdrop --db-pass=backdrop --db-host=$bee_test_db_host --auto");
    $this->assertStringContainsString('Backdrop installed successfully.', (string) $output_install);

    // Check site status after install.
    $output_after = shell_exec('bee status --site=install_test');
    $this->assertMatchesRegularExpression('/Site type +Multisite/', (string) $output_after);
    $this->assertMatchesRegularExpression('/Site directory +install_test/', (string) $output_after);
    $this->assertMatchesRegularExpression('/Database name +install_test/', (string) $output_after);
    $this->assertMatchesRegularExpression('/Database host +database/', (string) $output_after);

    // Cleanup the install.
    exec('bee --site=install_test db-drop -y');
    exec('rm -r sites/install_test/files');
    exec('cp settings.php sites/install_test');
  }
}
