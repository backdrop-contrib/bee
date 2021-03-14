<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console Install commands.
 */

use PHPUnit\Framework\TestCase;

class InstallCommandsTest extends TestCase {

  /**
   * Make sure that the install command works.
   */
  public function test_install_command_works() {
    // Check site status before install.
    $output_before = shell_exec('b status --site=install_test');
    print_r($output_before);
    // $this->assertStringContainsString('', $output);

    // Install the site.
    $output_install = shell_exec('b install --site=install_test --auto --db=mysql://backdrop:backdrop@database/install_test');
    print_r($output_install);

    // Check site status after install.
    $output_after = shell_exec('b status --site=install_test');
    print_r($output_after);
  }

}
