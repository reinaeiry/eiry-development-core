// Shorthand wrappers for the most common component search patterns.
// Wraps SCR_EntityHelper for common cases without requiring callers to
// know the SCR_EComponentFinderQueryFlags enum.
//
// Usage:
//   HealthComponent hp = HealthComponent.Cast(EIR_ComponentUtils.OnSelf(entity, HealthComponent));
//   InventoryComponent inv = InventoryComponent.Cast(EIR_ComponentUtils.InParents(entity, InventoryComponent));

class EIR_ComponentUtils
{
	//------------------------------------------------------------------------------------------------
	// Find a component on the entity itself only.
	static Managed OnSelf(notnull IEntity entity, typename componentType)
	{
		return entity.FindComponent(componentType);
	}

	//------------------------------------------------------------------------------------------------
	// Find a component on the entity or its direct parent.
	static Managed InParents(notnull IEntity entity, typename componentType)
	{
		Managed comp = entity.FindComponent(componentType);
		if (comp)
			return comp;

		IEntity parent = entity.GetParent();
		if (!parent)
			return null;

		return parent.FindComponent(componentType);
	}

	//------------------------------------------------------------------------------------------------
	// Find a component on the root-most parent in the hierarchy.
	static Managed OnRoot(notnull IEntity entity, typename componentType)
	{
		IEntity root = entity.GetRootParent();
		if (!root)
			return entity.FindComponent(componentType);

		return root.FindComponent(componentType);
	}

	//------------------------------------------------------------------------------------------------
	// Find a component by walking direct children (one level deep, not recursive).
	static Managed InChildren(notnull IEntity entity, typename componentType)
	{
		IEntity child = entity.GetChildren();
		while (child)
		{
			Managed comp = child.FindComponent(componentType);
			if (comp)
				return comp;

			child = child.GetSibling();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	// Find a component on the entity itself, then walk up parents until found.
	// Stops at the root. Returns null if not found anywhere in the chain.
	static Managed WalkUp(notnull IEntity entity, typename componentType)
	{
		IEntity current = entity;
		while (current)
		{
			Managed comp = current.FindComponent(componentType);
			if (comp)
				return comp;

			current = current.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	// Fills outComponents with all components of componentType found on direct children.
	// Returns the count found.
	static int CollectFromChildren(notnull IEntity entity, typename componentType, out array<Managed> outComponents)
	{
		if (!outComponents)
			outComponents = {};

		outComponents.Clear();

		IEntity child = entity.GetChildren();
		while (child)
		{
			Managed comp = child.FindComponent(componentType);
			if (comp)
				outComponents.Insert(comp);

			child = child.GetSibling();
		}

		return outComponents.Count();
	}
}
