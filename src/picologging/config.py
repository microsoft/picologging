import re
from logging.config import BaseConfigurator

import picologging
from picologging.handlers import MemoryHandler

IDENTIFIER = re.compile("^[a-z_][a-z0-9_]*$", re.I)


def valid_ident(s):
    m = IDENTIFIER.match(s)
    if not m:
        raise ValueError("Not a valid Python identifier: %r" % s)
    return True


def _resolve(name):
    """Resolve a dotted name to a global object."""
    name = name.split(".")
    used = name.pop(0)
    found = __import__(used)
    for n in name:
        used = used + "." + n
        try:
            found = getattr(found, n)
        except AttributeError:
            __import__(used)
            found = getattr(found, n)
    return found


def _handle_existing_loggers(existing, child_loggers, disable_existing):
    """
    When (re)configuring logging, handle loggers which were in the previous
    configuration but are not in the new configuration. There's no point
    deleting them as other threads may continue to hold references to them;
    and by disabling them, you stop them doing any logging.
    However, don't disable children of named loggers, as that's probably not
    what was intended by the user. Also, allow existing loggers to NOT be
    disabled if disable_existing is false.
    """
    root = picologging.root
    for log in existing:
        logger = root.manager.loggerDict[log]
        if log in child_loggers:
            logger.setLevel(picologging.NOTSET)
            logger.handlers = []
            logger.propagate = True
        else:
            logger.disabled = disable_existing


class DictConfigurator(BaseConfigurator):
    """
    Configure logging using a dictionary-like object to describe the
    configuration.
    """

    def configure(self):
        config = self.config
        if "version" not in config:
            raise ValueError("dictionary doesn't specify a version")
        if config["version"] != 1:
            raise ValueError("Unsupported version: %s" % config["version"])
        incremental = config.pop("incremental", False)
        if incremental:
            raise ValueError("Incremental option is not supported.")

        EMPTY_DICT = {}
        disable_existing = config.pop("disable_existing_loggers", True)

        # Do formatters first - they don't refer to anything else
        formatters = config.get("formatters", EMPTY_DICT)
        for name in formatters:
            try:
                formatters[name] = self.configure_formatter(formatters[name])
            except Exception as e:
                raise ValueError("Unable to configure " "formatter %r" % name) from e
        # Next, do filters - they don't refer to anything else, either
        filters = config.get("filters", EMPTY_DICT)
        for name in filters:
            try:
                filters[name] = self.configure_filter(filters[name])
            except Exception as e:
                raise ValueError("Unable to configure " "filter %r" % name) from e

        # Next, do handlers - they refer to formatters and filters
        # As handlers can refer to other handlers, sort the keys
        # to allow a deterministic order of configuration
        handlers = config.get("handlers", EMPTY_DICT)
        deferred = []
        for name in sorted(handlers):
            try:
                handler = self.configure_handler(handlers[name])
                handler.name = name
                handlers[name] = handler
            except Exception as e:
                if "target not configured yet" in str(e.__cause__):
                    deferred.append(name)
                else:
                    raise ValueError("Unable to configure handler " "%r" % name) from e

        # Now do any that were deferred
        for name in deferred:
            try:
                handler = self.configure_handler(handlers[name])
                handler.name = name
                handlers[name] = handler
            except Exception as e:
                raise ValueError("Unable to configure handler " "%r" % name) from e

        # Next, do loggers - they refer to handlers and filters

        # we don't want to lose the existing loggers,
        # since other threads may have pointers to them.
        # existing is set to contain all existing loggers,
        # and as we go through the new configuration we
        # remove any which are configured. At the end,
        # what's left in existing is the set of loggers
        # which were in the previous configuration but
        # which are not in the new configuration.
        root = picologging.root
        existing = list(root.manager.loggerDict.keys())
        # The list needs to be sorted so that we can
        # avoid disabling child loggers of explicitly
        # named loggers. With a sorted list it is easier
        # to find the child loggers.
        existing.sort()
        # We'll keep the list of existing loggers
        # which are children of named loggers here...
        child_loggers = []
        # now set up the new ones...
        loggers = config.get("loggers", EMPTY_DICT)
        for name in loggers:
            if name in existing:
                i = existing.index(name) + 1  # look after name
                prefixed = name + "."
                pflen = len(prefixed)
                num_existing = len(existing)
                while i < num_existing:
                    if existing[i][:pflen] == prefixed:
                        child_loggers.append(existing[i])
                    i += 1
                existing.remove(name)
            try:
                self.configure_logger(name, loggers[name])
            except Exception as e:
                raise ValueError("Unable to configure logger " "%r" % name) from e

        # Disable any old loggers. There's no point deleting
        # them as other threads may continue to hold references
        # and by disabling them, you stop them doing any logging.
        # However, don't disable children of named loggers, as that's
        # probably not what was intended by the user.
        _handle_existing_loggers(existing, child_loggers, disable_existing)

        # And finally, do the root logger
        root = config.get("root", None)
        if root:
            try:
                self.configure_root(root)
            except Exception as e:
                raise ValueError("Unable to configure root " "logger") from e

    def configure_formatter(self, config):
        """Configure a formatter from a dictionary."""
        if "()" in config:
            factory = config["()"]  # for use in exception handler
            try:
                result = self.configure_custom(config)
            except TypeError as te:
                if "'format'" not in str(te):
                    raise
                # Name of parameter changed from fmt to format.
                # Retry with old name.
                # This is so that code can be used with older Python versions
                # (e.g. by Django)
                config["fmt"] = config.pop("format")
                config["()"] = factory
                result = self.configure_custom(config)
        else:
            fmt = config.get("format", None)
            dfmt = config.get("datefmt", None)
            style = config.get("style", "%")
            cname = config.get("class", None)

            if not cname:
                c = picologging.Formatter
            else:
                c = _resolve(cname)

            # A TypeError would be raised if "validate" key is passed in with a formatter callable
            # that does not accept "validate" as a parameter
            if (
                "validate" in config
            ):  # if user hasn't mentioned it, the default will be fine
                result = c(fmt, dfmt, style, config["validate"])
            else:
                result = c(fmt, dfmt, style)

        return result

    def configure_filter(self, config):
        """Configure a filter from a dictionary."""
        if "()" in config:
            result = self.configure_custom(config)
        else:
            name = config.get("name", "")
            result = picologging.Filter(name)
        return result

    def add_filters(self, filterer, filters):
        """Add filters to a filterer from a list of names."""
        for f in filters:
            try:
                filterer.addFilter(self.config["filters"][f])
            except Exception as e:
                raise ValueError("Unable to add filter %r" % f) from e

    def configure_handler(self, config):
        """Configure a handler from a dictionary."""
        config_copy = dict(config)  # for restoring in case of error
        formatter = config.pop("formatter", None)
        if formatter:
            try:
                formatter = self.config["formatters"][formatter]
            except Exception as e:
                raise ValueError("Unable to set formatter " "%r" % formatter) from e
        level = config.pop("level", None)
        filters = config.pop("filters", None)
        if "()" in config:
            c = config.pop("()")
            if not callable(c):
                c = self.resolve(c)
            factory = c
        else:
            cname = config.pop("class")
            klass = self.resolve(cname)
            # Special case for handler which refers to another handler
            if issubclass(klass, MemoryHandler) and "target" in config:
                try:
                    th = self.config["handlers"][config["target"]]
                    if not isinstance(th, picologging.Handler):
                        config.update(config_copy)  # restore for deferred cfg
                        raise TypeError("target not configured yet")
                    config["target"] = th
                except Exception as e:
                    raise ValueError(
                        "Unable to set target handler " "%r" % config["target"]
                    ) from e
            # elif (
            #     issubclass(klass, picologging.handlers.SMTPHandler)
            #     and "mailhost" in config
            # ):
            #     config["mailhost"] = self.as_tuple(config["mailhost"])
            # elif (
            #     issubclass(klass, picologging.handlers.SysLogHandler)
            #     and "address" in config
            # ):
            #     config["address"] = self.as_tuple(config["address"])
            factory = klass
        props = config.pop(".", None)
        kwargs = {k: config[k] for k in config if valid_ident(k)}
        try:
            result = factory(**kwargs)
        except TypeError as te:
            if "'stream'" not in str(te):
                raise
            # The argument name changed from strm to stream
            # Retry with old name.
            # This is so that code can be used with older Python versions
            # (e.g. by Django)
            kwargs["strm"] = kwargs.pop("stream")
            result = factory(**kwargs)
        if formatter:
            result.setFormatter(formatter)
        if level is not None:
            result.setLevel(picologging._checkLevel(level))
        if filters:
            self.add_filters(result, filters)
        if props:
            for name, value in props.items():
                setattr(result, name, value)
        return result

    def add_handlers(self, logger, handlers):
        """Add handlers to a logger from a list of names."""
        for h in handlers:
            try:
                logger.addHandler(self.config["handlers"][h])
            except Exception as e:
                raise ValueError("Unable to add handler %r" % h) from e

    def common_logger_config(self, logger, config):
        """
        Perform configuration which is common to root and non-root loggers.
        """
        level = config.get("level", None)
        if level is not None:
            logger.setLevel(picologging._checkLevel(level))

        # Remove any existing handlers
        for h in logger.handlers[:]:
            logger.removeHandler(h)
        handlers = config.get("handlers", None)
        if handlers:
            self.add_handlers(logger, handlers)
        filters = config.get("filters", None)
        if filters:
            self.add_filters(logger, filters)

    def configure_logger(self, name, config):
        """Configure a non-root logger from a dictionary."""
        logger = picologging.getLogger(name)
        self.common_logger_config(logger, config)
        logger.disabled = False
        propagate = config.get("propagate", None)
        if propagate is not None:
            logger.propagate = propagate

    def configure_root(self, config):
        """Configure a root logger from a dictionary."""
        root = picologging.getLogger()
        self.common_logger_config(root, config)


dictConfigClass = DictConfigurator


def dictConfig(config):
    """Configure logging using a dictionary."""
    dictConfigClass(config).configure()
