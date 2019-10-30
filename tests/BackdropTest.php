<?php
/**
 * @file
 * PHPUnit tests for core Backdrop Console functionality.
 */

use PHPUnit\Framework\TestCase;

class CoreTest extends TestCase {

  /**
   * Make sure that `b` is installed correctly.
   */
  public function testBInstalledCorrectly() {
    // A '0' exit status means success.
    exec('b', $output, $result);
    $this->assertEquals(0, $result);
  }

  /**
   * Make sure that unknown commands display an error.
   */
  public function testUnknownCommandDisplaysError() {
    // Need to change this if we ever add a 'spoon' command ;-)
    $command = 'spoon';
    $output = shell_exec("b $command");
    $this->assertStringContainsString("There is no $command", $output);
  }

  /**
   * Make sure that required arguments are actually required.
   */
  public function testRequiredArgumentsAreRequired() {
    // `config-get` has a required argument: 'config_name'.
    $output = shell_exec('b config-get');
    $this->assertStringContainsString('Argument config_name is required', $output);
  }

  /**
   * Make sure that command aliases work.
   */
  public function testCommandAliasesWork() {
    // `st` is an alias for `core-status`.
    $output_command = shell_exec('b core-status');
    $output_alias = shell_exec('b st');
    $this->assertEquals($output_command, $output_alias);
  }

  /**
   * Make sure that the `--root` option works.
   *
   * Test running `b` from outside Backdrop without the '--root' option, and
   * then with the '--root' option.
   */
  public function testRootOptionWorks() {
    $output_no_root = shell_exec('cd ../ && b st');
    $this->assertStringContainsString('No Backdrop installation found', $output_no_root);

    $output_root = shell_exec('cd ../ && b --root=www st');
    $this->assertStringContainsString('Backdrop CMS Installation detected', $output_root);
  }

  /**
   * Make sure that the `--y/--yes` options work.
   */
  public function testYesOptionWorks() {
    $module = 'book';

    $output_y = shell_exec("b en --y $module");
    $this->assertStringContainsString("Module $module enabled", $output_y);

    $output_yes = shell_exec("b dis --yes $module");
    $this->assertStringContainsString("Module $module disabled", $output_yes);

    // Uninstall $module for future tests.
    exec("b pmu --y $module");
  }

}
