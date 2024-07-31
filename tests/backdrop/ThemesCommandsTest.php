<?php
/**
 * @file
 * PHPUnit tests for Bee Themes commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's commands for working with themes.
 */
class ThemesCommandsTest extends TestCase {

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
   * Make sure that the theme-enable/disable-debug commands work.
   */
  public function test_theme_debug_commands_works() {

    shell_exec('bee theme-enable-debug');
    $output = shell_exec('bee cget system.core theme_debug');
    $this->assertStringContainsString("1", (string) $output);

    shell_exec('bee theme-disable-debug');
    $output = shell_exec('bee cget system.core theme_debug');
    $this->assertStringContainsString("0", (string) $output);
  }

}
