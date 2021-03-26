<?php
/**
 * @file
 * PHPUnit tests for Bee PHP commands.
 */

use PHPUnit\Framework\TestCase;

class PHPCommandsTest extends TestCase {

  /**
   * Make sure that the eval command works.
   */
  public function test_eval_command_works() {
    // Test a simple command.
    $output_simple = shell_exec('bee eval \'echo "foo" . "bar";\'');
    $this->assertStringContainsString('foobar', $output_simple);

    // Test that the terminal semicolon is optional.
    $output_semicolon = shell_exec('bee eval \'echo "Hello world."\'');
    $this->assertStringContainsString('Hello world.', $output_semicolon);

    // Test that calling a Backdrop function works.
    exec('bee eval \'config_set("test.settings", "value", "foobar")\'');
    $output_backdrop = shell_exec('bee eval \'echo config_get("test.settings", "value")\'');
    $this->assertStringContainsString('foobar', $output_backdrop);
    
    // Cleanup test config file.
    exec('bee eval \'config("test.settings")->delete();\'');
  }

}
