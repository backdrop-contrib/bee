<?php
/**
 * @file
 * PHPUnit tests for Bee Theme commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's commands for working with themes.
 */
class ThemeCommandsTest extends TestCase {

  /**
   * Make sure that the theme-default command works.
   */
  public function test_theme_default_command_works() {
    $output = shell_exec('bee theme-default bartik');
    $this->assertStringContainsString("'Bartik' was set as the default theme.", (string) $output);

    // Reset theme.
    exec('bee theme-default basis');
    exec('bee disable bartik');
  }

  /**
   * Make sure that the theme-admin command works.
   */
  public function test_theme_admin_command_works() {
    $output = shell_exec('bee theme-admin basis');
    $this->assertStringContainsString("Basis' was set as the admin theme.", (string) $output);

    // Reset theme.
    exec('bee theme-admin seven');
  }

  /**
   * Make sure that the theme-debug command works.
   */
  public function test_theme_debug_command_works() {
    // Attempt to disable theme debug.
    $output = shell_exec('bee theme-debug FALSE');
    $this->assertStringContainsString('Theme debug is already disabled', (string) $output);

    // Enable theme debug.
    $output = shell_exec('bee theme-debug TRUE');
    $this->assertStringContainsString('Theme debug was enabled', (string) $output);

    // Get the status of theme debug.
    $output = shell_exec('bee theme-debug');
    $this->assertStringContainsString('Theme debug is enabled', (string) $output);

    // Disable theme debug.
    $output = shell_exec('bee theme-debug FALSE');
    $this->assertStringContainsString('Theme debug was disabled', (string) $output);

    // Get the status of theme debug.
    $output = shell_exec('bee theme-debug');
    $this->assertStringContainsString('Theme debug is disabled', (string) $output);
  }
}
